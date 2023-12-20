/************************************************************************************************************
 *
 *                                                    NV MANAGER
 *
************************************************************************************************************/
/*
 * nvm.h
 *
 *  Created on: Sep 21, 2019
 *  Author: Martin Patarinski
 *  Copyright: Open source. Further copyright shall be approved by the author
 *  Description:
 *   This is SW module to implement reading and writing into flash memory.
 *   Its goal is to minimize the erase cycles of the flash sectors and 
 *   this way preserve the reliability of the memory for a longer period.
 *   In embedded SW industry this is a well-known approach also known as 
 *   Flash Eeprom Emulation.
 */

/**********************************
* Inclusions
***********************************/

#include "nvm.h"
#include "nvm_cfg.h"

/**********************************
* Local variables
***********************************/
/* Data definition */
uint8_t NvmRamBuffer[NVM_BLOCK_MAX_SIZE] = {0};
uint8_t NvmGbcBuffer[NVM_BLOCK_MAX_SIZE] = {0};
NvmManagerDescriptor_t NvmManagerDescriptor = {0};
static uint16_t Nvm_blockCounter = 0;

/**********************************
* Local functions prototypes
***********************************/
static bool _getBlockInfo(uint32_t addr, NvmBlocksId_t* blockIdx, uint16_t* occCtr);
static void _garbageCollection(uint32_t pageAddr, uint8_t currentBlockIdx);
static bool _writeBytes(uint32_t addr, uint8_t *buf, uint16_t len);
static void _nvmCrc32(uint8_t* buffer, uint32_t bufferSize, uint32_t* calculatedCrc);
static bool _isNvmBlockEmpty(uint32_t addr, uint16_t size);

/**********************************
* Local functions definition
***********************************/
/**
* @brief    A function to get a block info from NVM
*
* @param    [in]addr : start address of the logical page
*           [out]blockIdx : index of the block in the configuration
*           [out]occCtr - occurece couter of the block
*
* @return   true if the information (header) of the block is extracted correctly, otherwise - false (perhaps no block is written)
*/
static bool _getBlockInfo(uint32_t addr, NvmBlocksId_t* blockIdx, uint16_t* occCtr)
{
    uint8_t blockHeader[BLOCK_HEADER_SIZE] = { 0 };
    uint32_t existingCrc = 0;
    uint32_t calcCrc = 0;
    uint16_t occCntr = 0;
    uint16_t blockPatt = 0;
    NvmBlocksId_t bIdx;
    bool bResult = false;

    FlsDrv_readBytes( addr, blockHeader, BLOCK_HEADER_SIZE );

    if(0 == memcmp(blockHeader, (uint8_t*)BLOCK_NOT_INIT, BLOCK_HEADER_SIZE))
    {
        /* unoccupied memory */
        return false;
    }

    memcpy((uint8_t*)&blockPatt, blockHeader, BLOCK_HEADER_HALF_SIZE);
    memcpy((uint8_t*)&occCntr, blockHeader+BLOCK_HEADER_HALF_SIZE, BLOCK_HEADER_HALF_SIZE);

    for(bIdx = (NvmBlocksId_t)0; bIdx < eNvmBlockCount; bIdx++)
    {
        if(blockPatt == NvmBlocks[bIdx].pattern)
        {
            *blockIdx = bIdx;
            *occCtr = occCntr;
            bResult = true;
            break;
        }
    }

    if(bResult == false)
    {
        /* invalid block found. Reset all NvM */
        NvmManagerDescriptor.bErrorDetected = true;
    }
    else
    {
        /* check CRC match  */
        FlsDrv_readBytes( addr, NvmRamBuffer, BLOCK_HEADER_SIZE+NvmBlocks[*blockIdx].size+NVM_CRC_LEN );
        memcpy(&existingCrc, NvmRamBuffer+BLOCK_HEADER_SIZE+NvmBlocks[*blockIdx].size, NVM_CRC_LEN);
        
        _nvmCrc32(NvmRamBuffer+BLOCK_HEADER_SIZE, NvmBlocks[*blockIdx].size, &calcCrc);
        
        if(existingCrc != calcCrc)
        {
            /* invalid CRC found. Reset all NvM */
            NvmManagerDescriptor.bErrorDetected = true;
        }
    }

    return bResult;
}

/**
* @brief    Check whether the logical block on a given address is empty or not
*
* @param    [in]addr : address of the header of the logical block
*           [in]size : the assumable size of the existin logical block
* 
* @return   true if the block is empty(not written after the last erase), otherwise - false
*/
static bool _isNvmBlockEmpty(uint32_t addr, uint16_t size)
{
    uint8_t tempBuffer[256];
    uint32_t counter  = 0;
    uint16_t remainingSize = 0;
    uint32_t currentAddress = 0;
    uint16_t currentSize = 0;
    bool isBlockErased = true;
    
    remainingSize = size;
    currentAddress = addr;
    
    
    if(remainingSize > 256)
    {
        currentSize = (uint16_t)256;
    }
    else
    {
        currentSize = remainingSize;
    }
    
    while(remainingSize > 256)
    {
        /* first check if this chunk is already written to the memory */
        FlsDrv_readBytes( currentAddress, tempBuffer, currentSize);
    
        /*check if all bytes are erased */
        for(counter = 0; counter < currentSize; counter++)
        {
            isBlockErased &= (0xFF == tempBuffer[counter]);
        }
        
        remainingSize -=  currentSize;
        currentAddress += currentSize;
    }
    
    if(remainingSize > 0)
    {
        /* first check if this chunk is already written to the memory */
        FlsDrv_readBytes( currentAddress, tempBuffer, remainingSize);
    
        /*check if all bytes are erased */
        for(counter = 0; counter < remainingSize; counter++)
        {
            isBlockErased &= (0xFF == tempBuffer[counter]);
        }
    }
    
    return isBlockErased;
}

/**
* @brief    Write the information (header) of a logical block from NVManager
*           OR Write the new data into the body of the logical block
*
* @param    [in]addr : address of the header of the logical block
*           [in]buf  : source buffer, that contains the data to be written
*           [in]len  : size of the data to be written
*
* @return   true if the writting was successful, otherwise - false
*/
static bool _writeBytes(uint32_t addr, uint8_t *buf, uint16_t len)
{
    bool result = false;
    
    /* check if we are not trying to write out of the boundaries */
    if((addr + len) < NVM_MANAGER_END_ADDR)
    {
       result = FlsDrv_writeBytes(addr, buf, len);
    }
    
    return result;
}

/**
* @brief    A function to take all data from a NVManager page and transfer it to the other NVManager page
*           A page or sector is considered to be te minimal eraseable size as per the specification of the Flash driver and the FLASH itself
*
* @param    [in]pageAddr : address of the current page
*           [in]currentBlockIdx : the block which is currently written
*
* @return   none
*/
static void _garbageCollection(uint32_t pageAddr, uint8_t currentBlockIdx)
{
    NvmBlocksId_t bIdx;
    uint16_t size;
    
    /* this allows the memory compare of the current block data while overtaking in the new page to be suppressed */
    NvmManagerDescriptor.bgarbageCollect = true;

    for(bIdx = (NvmBlocksId_t)0; bIdx < eNvmBlockCount; bIdx++)
    {
        if( (pageAddr < NvmBlocks[bIdx].readPointer) && (NvmBlocks[bIdx].readPointer < pageAddr+FLASH_SECTOR_SIZE) && (bIdx != currentBlockIdx))
        {
            nvm_read(bIdx, NvmGbcBuffer, &size);

            /* reset occurrence counter so that the latest data is always with higher occurrence number */
            NvmBlocks[bIdx].occurrenceCntr = 0;

            /* attention: Recursion here */
            nvm_write(bIdx, NvmGbcBuffer, NvmBlocks[bIdx].size);
        }
    }
    
    NvmManagerDescriptor.bgarbageCollect = false;
}

/**
* @brief    Performs calculation of checksum CRC32(helper function)
*
* @param    [in]buffer: buffer with data that will be checksummed
*           [in]bufferSize: size of the data into the buffer
*           [out]calculatedCrc: calculated CRC32
* 
* @return   none
*/
/*  */
static void _nvmCrc32(uint8_t* buffer, uint32_t bufferSize, uint32_t* calculatedCrc)
{
    uint32_t calcCrc = 0;

	calcCrc = CRC32_Calculate(buffer, bufferSize);
    
    *calculatedCrc = calcCrc;
}

/**********************************
* Interface functions definition
***********************************/

/**
* @brief    Initialize NVManager once after power-on. It searches the current read and write pointers and sets them into a structure in RAM
* 
* @param     none
*
* @return    none
*/
void nvm_init(void)
{
    uint8_t  pageHeader[PAGE_HEADER_SIZE] = { 0 };
    uint32_t idx;
    uint32_t currBlockAddr;
    uint16_t currOccCntr;
    NvmBlocksId_t bIdx;
    bool bWritePointerFound = false;
    bool bOpResult = false;

    memset(NvmRamBuffer, 0, NVM_BLOCK_MAX_SIZE);

    NvmManagerDescriptor.writePointer = 0;
    NvmManagerDescriptor.bIsInitialized = false;
    NvmManagerDescriptor.bErrorDetected = false;
    
    /* set the read point to not initialized */
    for(bIdx = (NvmBlocksId_t)0; bIdx < eNvmBlockCount; bIdx++)
    {
        NvmBlocks[bIdx].readPointer = READ_POINTER_NOT_SET;
    }

	/* go through all pages in te flash */
    for(idx=NVM_MANAGER_START_ADDR; idx<NVM_MANAGER_END_ADDR; idx+=FLASH_SECTOR_SIZE)
    {
        FlsDrv_readBytes( idx, pageHeader, PAGE_HEADER_SIZE );

        if(memcmp(pageHeader, (uint8_t*)PAGE_WRITTEN, PAGE_HEADER_SIZE) == 0)
        {
            /* page contains unprocessed data */
            NvmManagerDescriptor.writePointer = idx;
            bWritePointerFound = true;
        }
    }

    if(false == bWritePointerFound)
    {
        bOpResult = false;
        /* erase the whole flash page by page and start from scratch */
        for(idx=NVM_MANAGER_START_ADDR; idx< NVM_MANAGER_END_ADDR; idx += FLASH_SECTOR_SIZE)
        {
            bOpResult |= FlsDrv_eraseBlock4K(idx);
        }

        if(true == bOpResult)
        {
            NvmManagerDescriptor.writePointer = NVM_MANAGER_START_ADDR;
        }
        else
        {
            NvmManagerDescriptor.bErrorDetected = true;
        }
    }

    /* within the page that is currently written search for the first unoccupied NvM block 
	*  check if write pointer was found */
    if(false == bWritePointerFound)
    {
        /* locate the pointer at the first address after header of second Page */
        NvmManagerDescriptor.writePointer = NVM_MANAGER_START_ADDR + PAGE_HEADER_SIZE;

        /* erase page 0 */
        FlsDrv_eraseBlock4K(NVM_MANAGER_START_ADDR);
        /* set page header of the first page */
        FlsDrv_writeBytes( NVM_MANAGER_START_ADDR, (uint8_t*)PAGE_WRITTEN, PAGE_HEADER_HALF_SIZE);
        
#ifdef NVM_USE_DEFAULTS
        memcpy(NvmRamBuffer, nvmDefaults, DEFAULTS_SIZE);
        FlsDrv_writeBytes(DEFAULTS_START_ADDRESS , NvmRamBuffer, DEFAULTS_SIZE);
        NvmManagerDescriptor.writePointer += DEFAULTS_SIZE;
#endif
    }
    else
    {
        /* add offset so that write pointer is set to the first DR */
        NvmManagerDescriptor.writePointer += PAGE_HEADER_SIZE;

        currBlockAddr = NvmManagerDescriptor.writePointer;

        for(idx=PAGE_HEADER_SIZE; idx<FLASH_SECTOR_SIZE; )
        {
            if(false == _getBlockInfo(currBlockAddr, &bIdx, &currOccCntr))
            {
                /* first unoccupied block space found - this is the first possible writing address */
                NvmManagerDescriptor.writePointer = currBlockAddr;
                idx = FLASH_SECTOR_SIZE;
                continue;
            }

            currBlockAddr += (NvmBlocks[bIdx].size+BLOCK_HEADER_SIZE+NVM_CRC_LEN);
            idx += (NvmBlocks[bIdx].size+BLOCK_HEADER_SIZE+NVM_CRC_LEN);
        }
    }

    currBlockAddr = (NvmManagerDescriptor.writePointer&GET_FLASH_PAGE_MASK) + PAGE_HEADER_SIZE;
    
    /* Check if the rest of the page is empty : all bytes equal to 0xFF 
    *  if the rest of the page is not empty then error detected has to be true
    */
    if(true == _isNvmBlockEmpty(NvmManagerDescriptor.writePointer, FLASH_SECTOR_SIZE - (NvmManagerDescriptor.writePointer&GET_OFFSET_IN_PAGE_MASK)))
    {
        for(idx=PAGE_HEADER_SIZE; idx<FLASH_SECTOR_SIZE; )
        {
            if(false == _getBlockInfo(currBlockAddr, &bIdx, &currOccCntr))
            {
                /* it seems that no NVM blocks are stored on this NVM page */
                idx = FLASH_SECTOR_SIZE;
                continue;
            }
            
            /* restore the occurance counter and the read pointer from the readed NVM block info 
            *  biggest occurence counter for a block means the latest information stored in NVM for this block
            */
            if(currOccCntr > NvmBlocks[bIdx].occurrenceCntr)
            {
                NvmBlocks[bIdx].readPointer = currBlockAddr;
                NvmBlocks[bIdx].occurrenceCntr = currOccCntr;
            }

            /* switch to the adjacent NVM block on the same page and try to read out the info */
            currBlockAddr += (NvmBlocks[bIdx].size+BLOCK_HEADER_SIZE+NVM_CRC_LEN);
            idx += (NvmBlocks[bIdx].size+BLOCK_HEADER_SIZE+NVM_CRC_LEN);
        }
    }
    else
    {
        NvmManagerDescriptor.bErrorDetected = true;
    }
    
    if(true == NvmManagerDescriptor.bErrorDetected)
    {
        bOpResult = false;
        /* erase the whole FLASH page by page and start from scratch */
        for(idx=NVM_MANAGER_START_ADDR; idx<NVM_MANAGER_END_ADDR; idx += FLASH_SECTOR_SIZE)
        {
            bOpResult |= FlsDrv_eraseBlock4K(idx);
        }

        if(true == bOpResult)
        {
            NvmManagerDescriptor.writePointer = NVM_MANAGER_START_ADDR;
        }
        else
        {
            NvmManagerDescriptor.bIsInitialized = false;
            return;
        }
    }

    NvmManagerDescriptor.bIsInitialized = true;
}

/**
* @brief    Update data element in NVManager
*
* @param    [in]bIdx : index of the logical block to write the data
*           [in]data : pointer to the source data buffer
*           [in]size : size of the data to be written
* 
* @return   indicate if data is stored or an error has occurred
*/
bool nvm_write(const NvmBlocksId_t bIdx, const uint8_t* data, uint16_t size)
{
    uint32_t currPage;
    uint32_t nextPageAddr;
    uint32_t calculatedCrc32 = 0;
    bool writeResult = true;
    
    if( (NvmManagerDescriptor.bIsInitialized == false) || (bIdx >= eNvmBlockCount) )
    {
        return false;
    }
    
    /* first check if there is a change of the parameter to be written. This is done to decrease the number fo writings into FLASH */
    if( (READ_POINTER_NOT_SET != NvmBlocks[bIdx].readPointer) &&
        (true == FlsDrv_readBytes( NvmBlocks[bIdx].readPointer, (uint8_t*)NvmRamBuffer, NvmBlocks[bIdx].size + BLOCK_HEADER_SIZE)) )
    {
        /* perform this chech only if no garbage collection is ongoing */
        if((false == NvmManagerDescriptor.bgarbageCollect)&&(0 == memcmp(data, NvmRamBuffer+BLOCK_HEADER_SIZE, NvmBlocks[bIdx].size)))
        {
            /* the data is already stored */
            return true;
        }
    }

    if( (NvmManagerDescriptor.writePointer%GET_OFFSET_IN_PAGE_MASK + NvmBlocks[bIdx].size + BLOCK_HEADER_SIZE + NVM_CRC_LEN) > FLASH_SECTOR_SIZE )
    {
        /* page overflow */
        currPage = NvmManagerDescriptor.writePointer&GET_FLASH_PAGE_MASK;

        if(NvmManagerDescriptor.writePointer + FLASH_SECTOR_SIZE > NVM_MANAGER_END_ADDR)
        {
            nextPageAddr = NVM_MANAGER_START_ADDR;
        }
        else
        {
            nextPageAddr = (NvmManagerDescriptor.writePointer&GET_FLASH_PAGE_MASK) + FLASH_SECTOR_SIZE;
        }

        NvmManagerDescriptor.writePointer = nextPageAddr + PAGE_HEADER_SIZE;

        /* erase next page */
        FlsDrv_eraseBlock4K(nextPageAddr);

        /* mark next page as written */
        writeResult &= _writeBytes( nextPageAddr, (uint8_t*)PAGE_MARK_AS_WRITTEN, PAGE_HEADER_HALF_SIZE);

        /* ensure that all NvM blocks are updated in the next page */
        _garbageCollection(currPage, bIdx);
        
        /* set the current occurance counter to 0 */
        NvmBlocks[bIdx].occurrenceCntr = 0;

        /* mark this page as ready to be erased */
        writeResult &= _writeBytes( currPage+PAGE_HEADER_HALF_SIZE, (uint8_t*)PAGE_MARK_AS_READ, PAGE_HEADER_HALF_SIZE);
    }

    memset(NvmRamBuffer, 0, NVM_BLOCK_MAX_SIZE);

    memcpy(NvmRamBuffer, (uint8_t*)(&NvmBlocks[bIdx].pattern), BLOCK_HEADER_HALF_SIZE);

    /* Counter is reset on Garbage collection */
    NvmBlocks[bIdx].occurrenceCntr++;
    
    /* Set the occurance counter  */
    memcpy(NvmRamBuffer+BLOCK_HEADER_HALF_SIZE, (uint8_t*)(&NvmBlocks[bIdx].occurrenceCntr), BLOCK_HEADER_HALF_SIZE);

    /* copy the block data */
    memcpy(NvmRamBuffer+BLOCK_HEADER_SIZE, data, NvmBlocks[bIdx].size);
    
    /* add checksum of the data to be written */
    _nvmCrc32(NvmRamBuffer+BLOCK_HEADER_SIZE, NvmBlocks[bIdx].size, &calculatedCrc32);
    
    memcpy((uint8_t*)(NvmRamBuffer+BLOCK_HEADER_SIZE+NvmBlocks[bIdx].size), &calculatedCrc32, NVM_CRC_LEN);
    
    /* In case of error delete the whole NVM area and force the default settings 
    * Normally false should never happen if NVM is initialized correctly. OTherwise the NVM content can not be trust any more
    */
    if((true == writeResult)&&(true == _writeBytes( NvmManagerDescriptor.writePointer, (uint8_t*)NvmRamBuffer, NvmBlocks[bIdx].size + BLOCK_HEADER_SIZE + NVM_CRC_LEN)))
    {
        NvmBlocks[bIdx].readPointer = NvmManagerDescriptor.writePointer;

        NvmManagerDescriptor.writePointer += (NvmBlocks[bIdx].size + BLOCK_HEADER_SIZE + NVM_CRC_LEN);

        if( 0 == (NvmManagerDescriptor.writePointer%FLASH_SECTOR_SIZE) )
        {
            /* page overflow will be performed on the next write operation */
            NvmManagerDescriptor.writePointer -= BLOCK_HEADER_HALF_SIZE;
        }
        
        return true;
    }
    else
    {
        /* NVM writing was not successful - perform reinitialization of the NVM */
        bool bOpResult = false;
        uint32_t idx = 0;
        
        /* erase the whole logical page by page and start from scratch */
        for(idx=NVM_MANAGER_START_ADDR; idx<NVM_MANAGER_END_ADDR; idx += FLASH_SECTOR_SIZE)
        {
            bOpResult |= FlsDrv_eraseBlock4K(idx);
        }

        if(true == bOpResult)
        {
            NvmManagerDescriptor.writePointer = NVM_MANAGER_START_ADDR;
        }
        else
        {
            NvmManagerDescriptor.bIsInitialized = false;
        }
        
        /* set the read point to not initialized */
        for(idx = (NvmBlocksId_t)0; idx < eNvmBlockCount; idx++)
        {
            NvmBlocks[idx].readPointer = READ_POINTER_NOT_SET;
        }
        
        return false;
    }
}

/**
* @brief    Read data element from NVManager
*
* @param    [in]bIdx : index of the logical block to write the data
*           [out]data : pointer to the source data buffer
*           [out]size : pointer size of the data that was read
* 
* @return   true if data is read correctly. Otherwise - false
*/
bool nvm_read(const NvmBlocksId_t bIdx, uint8_t* data, uint16_t *size)
{
    uint32_t existingCrc32 = 0;
    uint32_t calculatedCrc32 = 0;
    bool bResL = false;

    if( (NvmManagerDescriptor.bIsInitialized == false) || (bIdx >= eNvmBlockCount) || (READ_POINTER_NOT_SET == NvmBlocks[bIdx].readPointer) )
    {
        return false;
    }
    
    bResL = FlsDrv_readBytes( NvmBlocks[bIdx].readPointer, 
                             (uint8_t*)NvmRamBuffer, 
                             NvmBlocks[bIdx].size + BLOCK_HEADER_SIZE + NVM_CRC_LEN );

    if(bResL == true)
    {
        memcpy( &existingCrc32, 
                (uint8_t*)(NvmRamBuffer + NvmBlocks[bIdx].size + BLOCK_HEADER_SIZE), 
                NVM_CRC_LEN );
        _nvmCrc32(NvmRamBuffer+BLOCK_HEADER_SIZE, NvmBlocks[bIdx].size, &calculatedCrc32);
        
        if (calculatedCrc32 == existingCrc32)
        {
            *size = NvmBlocks[bIdx].size;
            memcpy(data, (uint8_t*)(NvmRamBuffer+BLOCK_HEADER_SIZE), NvmBlocks[bIdx].size);
            bResL = true;
        }
    }

    return bResL;
}

/**
* @brief    Get error status of the NVManager
*
* @param    none
* 
* @return   true if there is an error detected. Otherwise - false
*/
bool nvm_get_error(void)
{
    return NvmManagerDescriptor.bErrorDetected;
}


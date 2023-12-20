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
 *   this way preserve the reliability of the memory for a longer period 
 */

#ifndef __NVM_H_
#define __NVM_H_

/**********************************
* Inclusions
***********************************/
#include "nvm_cfg.h"

#define 	FLASH_PAGE_MASK1				0xFFFFF000
#define 	FLASH_PAGE_MASK2				0xFFF

#define LOGICAL_PAGE_SIZE		    4096u
#define GET_FLASH_PAGE_MASK			FLASH_PAGE_MASK1
#define GET_OFFSET_IN_PAGE_MASK 	FLASH_PAGE_MASK2

#define READ_POINTER_NOT_SET        0xFFFFFFFF

/**********************************
* Type definitions
***********************************/
enum flashStates
{
    eNotInit,
    eInit, 
    eCount
};

typedef enum
{
    NVM_INIT = 0,
    NVM_PROCESSING,
    NVM_COMPLETED
} eErrorNvmCodesType;

typedef struct
{
	uint32_t writePointer;
	bool bIsInitialized;
	bool bErrorDetected;
    bool bgarbageCollect;
} NvmManagerDescriptor_t;

/**********************************
* Data declarations
***********************************/
extern NvmManagerDescriptor_t NvmManagerDescriptor;
extern uint8_t NvmRamBuffer[NVM_BLOCK_MAX_SIZE];

/**********************************
* Interface
***********************************/

/**
* @brief    Initialize NVManager once after power-on. It searches the current read and write pointers and sets them into a structure in RAM
* 
* @param     none
*
* @return    none
*/
void nvm_init(void);

/**
* @brief    Update data element in NVManager
*
* @param    [in]bIdx : index of the logical block to write the data
*           [in]data : pointer to the source data buffer
*           [in]size : size of the data to be written
* 
* @return   indicate if data is stored or an error has occurred
*/
bool nvm_write(const NvmBlocksId_t bIdx, const uint8_t* data, uint16_t size);

/**
* @brief    Read data element from NVManager
*
* @param    [in]bIdx : index of the logical block to write the data
*           [out]data : pointer to the source data buffer
*           [out]size : pointer size of the data that was read
* 
* @return   true if data is read correctly. Otherwise - false
*/
bool nvm_read(const NvmBlocksId_t bIdx, uint8_t* data, uint16_t *size);

/**
* @brief    Get error status of the NVManager
*
* @param    none
* 
* @return   true if there is an error detected. Otherwise - false
*/
bool nvm_get_error(void);
   

#endif /* __NVM_H_ */

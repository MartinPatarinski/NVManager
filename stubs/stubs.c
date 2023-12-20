
/*
 * stubs.c
 *
 *  Created on: Sep 15, 2019
 *  Author: Martin Patarinski
 *  Copyright: Open source. Further copyright shall be approved by the author
 *  Description:
 *  This is a stub file where the required interfaces of the NVManager SW component are implemented for Unit test environment.
 *  The Flash driver APIs would have to be replaced, wrapped or adapted, if the NVManager is integrated into an embedded project.
 */

#include "stubs.h"

/* This is a large buffer into the RAM of the PC in order to simulate a FLASH of an embedded device. Only for Unit test purpose */
uint8_t FlashSimu[0x800][BUFF_FLASH_PAGE_SIZE]; /* 8MB */

/* This is a significantly big buffer, that would be located in the RAM of the embedded device. It is required by the NVManager in order to operate
   It has to be configured according to the specification of the used FLASH memoy and the used Flash driver. 
   The macro BUFF_FLASH_PAGE_SIZE shall be set with the minimum erasable sector in the FLASH memory */
uint8_t sector_buffer[BUFF_FLASH_PAGE_SIZE] = { 0 };

/* A table for CRC calculation. Only for Unit test. Assuming there would be a library or HW module for CRC calculation on the Embedded project */
uint32_t Crc32_table[256];

static void generate_table(uint32_t table[256]);
static uint32_t update(uint32_t table[256], uint32_t initial, const void* buf, size_t len);

/**********************************************************  
                    INTERFACE FUNCTIONS
 *********************************************************/
/* A dummy implementation of the initialization of the underalying flash driver */
void FlsDrv_Init()
{
	/* set the FLASH as erased */
	memset(FlashSimu, 0xFF, 0x800*BUFF_FLASH_PAGE_SIZE);

	/* initialize the CRC table so that it is ready for calculation */
 	generate_table(Crc32_table);
}

/* A dummy implementation of the reading function of the flash driver */
bool FlsDrv_readBytes( uint32_t addr, uint8_t* dest, uint32_t len)
{
	uint32_t page = (addr & FLASH_PAGE_MASK1) / BUFF_FLASH_PAGE_SIZE;
	uint32_t offset = addr & FLASH_PAGE_MASK2;

	memcpy(dest, &FlashSimu[page][offset], len);

	return true;
}

/* A dummy implementation of the erasing function of the flash driver, that erases one physical block */
bool FlsDrv_eraseBlock4K(uint32_t addr)
{
	uint32_t page = (addr & FLASH_PAGE_MASK1) / BUFF_FLASH_PAGE_SIZE;

	memset(&FlashSimu[page][0], 0xFF, BUFF_FLASH_PAGE_SIZE);

	return true;
}

/* A dummy implementation of the writing function of the flash driver */
bool FlsDrv_writeBytes( uint32_t addr, uint8_t* src, uint32_t len)
{
	uint32_t page = (addr & FLASH_PAGE_MASK1) / BUFF_FLASH_PAGE_SIZE;
	uint32_t offset = addr & FLASH_PAGE_MASK2;

	memcpy(&FlashSimu[page][offset], src, len);

	return true;
}

/* A dummy implementation of the erasing function of the flash driver, that erases the whole data FLASH (memory area that is used by the NVManager) */
bool FlsDrv_chipErase(void)
{
	memset(FlashSimu, 0xFF, sizeof(FlashSimu));

	return true;
}

/* A dummy implementation of the CRC32 calculation function */
uint32_t CRC32_Calculate(uint8_t* buffer, uint32_t bufferSize)
{
	uint32_t crc = 0;
	
	crc = update(Crc32_table, 0, buffer, bufferSize);
	
	return crc;
}

/**********************************************************  
                    LOCAL FUNCTIONS
 *********************************************************/
/* A helper function to generate a table for CRC32 calculation. 
   Only for Unit test. Assuming there would be a library or HW module for CRC calculation on the Embedded project */
static void generate_table(uint32_t table[256])
{
	uint32_t polynomial = 0xEDB88320;
	for (uint32_t i = 0; i < 256; i++) 
	{
		uint32_t c = i;
		for (size_t j = 0; j < 8; j++) 
		{
			if (c & 1) {
				c = polynomial ^ (c >> 1);
			}
			else {
				c >>= 1;
			}
		}
		table[i] = c;
	}
}

/* A helper function to calculate CRC32
   Only for Unit test. Assuming there would be a library or HW module for CRC calculation on the Embedded project */
static uint32_t update(uint32_t table[256], uint32_t initial, const void* buf, size_t len)
{
	uint32_t c = initial ^ 0xFFFFFFFF;
	const uint8_t* u = (const uint8_t*)(buf);
	for (size_t i = 0; i < len; ++i) 
	{
		c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
	}
	return c ^ 0xFFFFFFFF;
}

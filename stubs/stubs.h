/*
 * stubs.h
 *
 *  Created on: Sep 15, 2019
 *  Author: Martin Patarinski
 *  Copyright: Open source. Further copyright shall be approved by the author
 *  Description:
 *  This is a stub file where the required interfaces of the NVManager SW component are implemented for Unit test environment.
 *  The Flash driver APIs would have to be replaced, wrapped or adapted, if the NVManager is integrated into an embedded project.
 */

#ifndef STUBS_H_
#define STUBS_H_

/**********************************************************  
                    INCLUSIONS
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**********************************************************  
            PREPROCESSOR DEFINITIONS
 *********************************************************/
#define DEBUG

#define MARISSD

#define BUFF_FLASH_PAGE_SIZE 0x1000

#define true 1
#define false 0

#define TOTAL_FLASH_SIZE 0x40000

#define FLASH_PAGE_MASK1 0xFFFFF000
#define FLASH_PAGE_MASK2 0x00000FFF

/**********************************************************  
                    INTERFACE TYPES
 *********************************************************/
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned char bool;

/**********************************************************  
                    GLOBAL VARIABLES
 *********************************************************/
extern uint8_t FlashSimu[0x800][BUFF_FLASH_PAGE_SIZE];
extern uint8_t sector_buffer[BUFF_FLASH_PAGE_SIZE];

/**********************************************************  
                    INTERFACE FUNCTIONS
 *********************************************************/
extern void FlsDrv_Init();

extern bool FlsDrv_readBytes( uint32_t addr, uint8_t* dest, uint32_t len);

extern bool FlsDrv_eraseBlock4K(uint32_t addr);

extern bool FlsDrv_writeBytes( uint32_t addr, uint8_t* src, uint32_t len);

extern bool FlsDrv_chipErase(void);

extern uint32_t CRC32_Calculate(uint8_t* buffer, uint32_t bufferSize);

#endif /* STUBS_H_ */

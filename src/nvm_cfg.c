/*
 * nvm_cfg.c
 *
 *  Created on: Sep 21, 2019
 *  Author: Martin Patarinski
 *  Copyright: Open source. Further copyright shall be approved by the author
 *  Description:
 *  This is a configuration file where the configuration of the NVManager is set
 */

/**********************************************************  
                INCLUSIONS
 *********************************************************/
#include "nvm_cfg.h"

/**********************************************************  
                    GLOBAL VARIABLES
 *********************************************************/

/* A descriptor of all used logical blocks 
 * Every logical block has an ID pattern (2 bytes), size (4 bytes) and info of the pointer where the last copy of the block is located (address and occurence counter)
 * When this descriptor is edited, don't forget to edit also the enumeration NvmBlocksId_t!!!
 */
BlockDescriptor_t NvmBlocks[eNvmBlockCount] =
{
    { 0xCC01, NVM_BLOCK_1_SIZE, 0x00000000, 0x0000 },
    { 0xCC02, NVM_BLOCK_2_SIZE, 0x00000000, 0x0000 },
    { 0xCC03, NVM_BLOCK_3_SIZE, 0x00000000, 0x0000 },
    { 0xAA04, NVM_BLOCK_4_SIZE, 0x00000000, 0x0000 },
    { 0xAA05, NVM_BLOCK_5_SIZE, 0x00000000, 0x0000 },
    { 0xAA06, NVM_BLOCK_6_SIZE, 0x00000000, 0x0000 },
    { 0xAA07, NVM_BLOCK_7_SIZE, 0x00000000, 0x0000 },
    { 0xAA08, NVM_BLOCK_8_SIZE, 0x00000000, 0x0000 },
    { 0xAA09, NVM_BLOCK_9_SIZE, 0x00000000, 0x0000 },
    { 0xAA10, NVM_BLOCK_10_SIZE, 0x00000000, 0x0000 },
    { 0xAA11, NVM_BLOCK_11_SIZE, 0x00000000, 0x0000 },
    { 0xAA12, NVM_BLOCK_12_SIZE, 0x00000000, 0x0000 },
	{ 0xAA13, NVM_BLOCK_13_SIZE, 0x00000000, 0x0000 },
	{ 0xAA14, NVM_BLOCK_14_SIZE, 0x00000000, 0x0000 },
	{ 0xAA15, NVM_BLOCK_15_SIZE, 0x00000000, 0x0000 }
};

#ifdef NVM_USE_DEFAULTS
uint8_t nvmDefaults[DEFAULTS_SIZE] = 
{
    0x03, 0xCC, 0x01, 0x00, 0x00, 0x00, 0xFA, 0x42, 0x00, 0x00, 0x20, 0xC2, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF
};
#endif

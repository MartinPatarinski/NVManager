/*
 * nvm_cfg.h
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
#ifndef UNIT_TEST_BLACK_BOX
#include "../stubs/FlsDrv.h"
#else
#include "../stubs/stubs.h"
#endif

#ifndef __NVM_CFG_H_
#define __NVM_CFG_H_

/**********************************************************  
            PREPROCESSOR DEFINITIONS
 *********************************************************/
//#define NVM_USE_DEFAULTS

#define NUMBER_OF_4KB_FLASH_SECTORS 256
#define PAGE_HEADER_SIZE            4
#define PAGE_HEADER_HALF_SIZE       2
#define PAGE_HEADER_ONE_BYTE       	1
#define PAGE_NOT_INIT               "\xFF\xFF\xFF\xFF" // flash memory is erased
#define PAGE_WRITTEN                "\xAA\x55\xFF\xFF" // a new data is written into the page and we shall preserve it unitl it is read
#define PAGE_READ                   "\xAA\x55\xFF\xAA" // the data in the page is read and processed. It can be erased
#define PAGE_OLDEST					"\xAA\x55\x55\xFF" // Mark that data to be read first after init
#define PAGE_MARK_AS_READ           "\xAA"         	   // mark page as read
#define PAGE_MARK_AS_WRITTEN        "\xAA\x55"         // mark page as written
#define PAGE_MARK_AS_LAST			"\x55"			   // mark as oldest

#define DR_HEADER_SIZE              4
#define DR_HEADER_HALF_SIZE         2
#define DR_NOT_INIT                 "\xFF\xFF" // data record is erased
#define DR_WRITTEN                  "\x33\xFF" // a new data is written into the page and we shall preserve it unitl it is read
#define DR_READ                     "\x33\xDD" // the data in the page is read and processed. It can be erased
#define DR_MARK_AS_READ             "\xDD"     // mark page as read
#define DR_MARK_AS_WRITTEN          "\x33"     // mark page as written

#define BLOCK_HEADER_SIZE			4
#define BLOCK_HEADER_HALF_SIZE		2
#define BLOCK_NOT_INIT              "\xFF\xFF\xFF\xFF" // flash memory is erased

#define MAX_DR_SIZE 		        0x200

#define NVM_MANAGER_START_ADDR      0x00002000
#define NVM_MANAGER_END_ADDR        0x00004000

#define FLASH_SECTOR_SIZE           0x1000

/* Change block patterns, when you change block sizes! */
#define NVM_BLOCK_1_SIZE            0x13C  //Log
#define NVM_BLOCK_2_SIZE            0x0A   //power on data
#define NVM_BLOCK_3_SIZE            0x1E   //Keypad counter 1
#define NVM_BLOCK_4_SIZE            0x1E   //Keypad counter 2
#define NVM_BLOCK_5_SIZE            0x1E   //Keypad counter 3
#define NVM_BLOCK_6_SIZE            0x0C   //Keypad counter
#define NVM_BLOCK_7_SIZE            0x0F   //Temperature
#define NVM_BLOCK_8_SIZE            0x28   
#define NVM_BLOCK_9_SIZE            0x0A   //Voltage
#define NVM_BLOCK_10_SIZE           0x14
#define NVM_BLOCK_11_SIZE           0x0A
#define NVM_BLOCK_12_SIZE           0x02   
#define NVM_BLOCK_13_SIZE           0xFF
#define NVM_BLOCK_14_SIZE           0x04   //Write cycle counter
#define NVM_BLOCK_15_SIZE           0x40

#define NVM_BLOCK_MAX_SIZE          (NVM_CRC_LEN+NVM_BLOCK_1_SIZE) // max size should be bigger or equal to the size of the biggest NvM block

#define NVM_CRC_LEN                 0x04

#define ram_buffer                  FlsDrv_sector_buffer
#define RAM_BUFF_SIZE               (MAX_DR_SIZE + PAGE_HEADER_SIZE + DR_HEADER_SIZE)

/**********************************************************  
                    INTERFACE TYPES
 *********************************************************/
typedef enum sNvmBlocksId
{
	eNvmBlock1,  
	eNvmBlock2,     //power on data
	eNvmBlock3,     //Keypad counter GR1
    eNvmBlock4,     //Keypad counter GR2
    eNvmBlock5,     //Keypad counter GR3
    eNvmBlock6,     //Keypad counter Tea
    eNvmBlock7,     //Temperature
    eNvmBlock8,     //Keypad dose GR1 GR3 GR3
    eNvmBlock9,     //Keypad dose tea
    eNvmBlock10,    //Services
    eNvmBlock11,    //Alarms
    eNvmBlock12,    //FLag - first time connect to mqtt
	eNvmBlock13,    //NTP server - string
	eNvmBlock14,    //NTP server port - string
	eNvmBlock15,	//Latest AWS job id which is used for FOTA update - string
	eNvmBlockCount
} NvmBlocksId_t;

/* A type for the descriptor of all used logical blocks */
typedef struct
{
    const uint16_t pattern; /* ID pattern for searching */
	const uint32_t size; /* maximal size of the written data */
    uint32_t readPointer; /* address of the instance of the logical block */
    uint16_t occurrenceCntr; /* occurence counter of the instance */
} BlockDescriptor_t;

/**********************************************************  
                    GLOBAL VARIABLES
 *********************************************************/
/* A descriptor of all used logical blocks 
 * Every logical block has an ID pattern (2 bytes), size (4 bytes) and info of the pointer where the last copy of the block is located (address and number of occurencies)
 * When this descriptor is edited, don't forget to edit also the enumeration NvmBlocksId_t!!!
 */
extern BlockDescriptor_t NvmBlocks[eNvmBlockCount];

#ifdef NVM_USE_DEFAULTS
  #define DEFAULTS_SIZE               (NVM_BLOCK_1_SIZE+BLOCK_HEADER_SIZE)
  #define DEFAULTS_START_ADDRESS      (NVM_MANAGER_START_ADDR+PAGE_HEADER_SIZE)
  extern uint8_t nvmDefaults[DEFAULTS_SIZE];
#endif

#endif /* __NVM_CFG_H_ */

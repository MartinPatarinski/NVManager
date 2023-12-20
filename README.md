# NVManager

A manager of the non-volatile memory that is used for storing user data or parameters

Author: Martin Patarinski
Copyright: Open source. Further copyright shall be approved by the author
Description:
   This is SW module to implement reading and writing into flash memory.
   Its goal is to minimize the erase cycles of the flash sectors and 
   this way preserve the reliability of the memory for a longer period.
   In embedded SW industry this is a well-known approach also known as 
   Flash Eeprom Emulation.

The SW component NVManager has a universal interface so that it is easy for integration into any embedded project

The SW component NVManager has syncronous APIs for reading and writing. Asynchronous will be added in the next stage, therefore for RTOS embedded applications the timing constraints have to calculated additionally

The NVManager uses recursion to perform a garbage collection. This operation is necessary when the physical memory is over and new memory has to be freed

# Integration
The NVManager has to be configured carefully so that all the required non-volatile parameters are grouped into blocks. The good practice is to have the data, that is written more often into separate block(s)
The mandatory fields for configuration are: 
NvmBlocksId_t
NvmBlocks
MAX_DR_SIZE
NVM_MANAGER_START_ADDR
NVM_MANAGER_END_ADDR
FLASH_SECTOR_SIZE
LOGICAL_PAGE_SIZE

All of the required interfaces have to be implemented, wrapped or adapted according to the used HW platform and used FLASH memory (i.e. STM32, ESP32, etc.)

# Unit test
The unit test is designed in ANSI C. Its purpose is to test the SW component NVManager as a black box. It is a simple and self-sufficient environment without any dependencies of third-party libraries or frameworks. Its sole purpose is to test the code, but can be improved to provide statistics such as code coverage etc.

# Constraints
In order to guarantee compatibility with older versions of the NVManager, the user must always change the block patterns in case the block sizes have been changed

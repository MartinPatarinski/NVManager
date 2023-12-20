/*
 ============================================================================
 Name        : unit_test_main.c
 Author      : Martin Patarinski
 Version     :
 Copyright   : Open source. Further copyright shall be approved by the author
 Description : Unit test in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "stubs/stubs.h"
#include "src/nvm.h"

#define LOAD_PREVIOUS_FLASH

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define UT_CHECK(exp) { \
	if(exp) {printf("%s Check passed!", KGRN);} else {TestFailedCounter++; printf("%s Check FAILED!", KRED);} TestCounter++; printf("%s\n", KNRM);\
}

static uint8_t testData[MAX_DR_SIZE];
static uint16_t testDataReadSize = 0;
static uint32_t TestCounter = 0;
static uint32_t TestFailedCounter = 0;

void printDR(uint8_t* buff, uint16_t len)
{
	uint32_t idx = 0;

	printf("Data record: \n");
	for(idx=0; (idx<len)&&(idx<0x32) ; idx++)
	{
		printf("%02X ", buff[idx]);
	}
	printf("\n");
}

void fillWithRandom(uint8_t* pBuff, uint32_t size)
{
	uint32_t randNum = 0;
	uint32_t rndCtr = 0;
	int randRes = 0;

	if (pBuff == NULL)
	{
		printf("%s Error in fillWithRandom()! NULL pointer passed. %s \n", KRED, KNRM);
		return;
	}

	/*if (size < 4 || size%4 != 0)
	{
		printf("%s Error in fillWithRandom()! The provided size is either zero or not alligned to 4. %s \n", KRED, KNRM);
		return;
	}*/

	while(rndCtr < (size+4))
	{
		randRes = rand();
		randNum = (uint32_t)randRes;
		pBuff[rndCtr++] = (uint8_t)(randNum>>24);
		pBuff[rndCtr++] = (uint8_t)(randNum>>16);
		pBuff[rndCtr++] = (uint8_t)(randNum>>8);
		pBuff[rndCtr++] = (uint8_t)(randNum);
	}

	/* fill the last bytes */
	while(rndCtr<size)
	{
		randNum = (uint32_t)rand();
		pBuff[rndCtr++] = (uint8_t)(randNum);
	}
}

/* Test normal behavior of the SWC NVManager */
void TestCase1(void)
{
	printf("\n");
	printf("Name: Test case 1\n");
	printf("  Description: Test initialization\n");
	printf("  Preconditions: none\n");
	printf("  Test steps: Invoke nvm_init\n");
	printf("  Check results: No crashes or other errors are reported\n");
	printf("  Post steps: none\n");

	bool nvmRes = true;

	nvmRes &= nvm_read(eNvmBlock2, testData, &testDataReadSize);
	printf("\n	* Checking whether the NVManager accepted a read request when not initialized... ");
	UT_CHECK(false == nvmRes)

	fillWithRandom(testData, NVM_BLOCK_2_SIZE);
	nvmRes &= nvm_write(eNvmBlock2, testData, NVM_BLOCK_2_SIZE);
	printf("\n	* Checking whether the NVManager accepted a write request when not initialized... ");
	UT_CHECK(false == nvmRes)	

	printf("\n	* Initializing the NVManager...");
	nvm_init();

	printf("\n	* Checking if there are any errors detected during the initialization... ");
	bool bRes = nvm_get_error();
	UT_CHECK(!bRes)
	printf("\n");
}

/* Test normal behavior of the SWC NVManager */
void TestCase2(void)
{
	printf("\n");
	printf("Name: Test case 2\n");
	printf("  Description: Test normal behavior, simple operations\n");
	printf("  Preconditions: The NVManager is initialized\n");
	printf("  Test steps: Write data and then read it back\n");
	printf("  Check results: The read data is the same as the one provided for writing\n");
	printf("  Post steps: none\n");

	uint8_t testDataRead[NVM_BLOCK_2_SIZE];
	uint8_t result = 0xFF;
	bool nvmRes = true;

	testDataReadSize = 0;
	fillWithRandom(testData, NVM_BLOCK_2_SIZE);
	nvmRes = true;
	nvmRes &= nvm_write(eNvmBlock2, testData, NVM_BLOCK_2_SIZE);
	nvmRes &= nvm_read(eNvmBlock2, testDataRead, &testDataReadSize);

	printf("\n	* Checking whether the size of the read data is correct... ");
	UT_CHECK(NVM_BLOCK_2_SIZE == testDataReadSize)
	printf("\n	* Checking whether the NVManager accepted the requests... ");
	UT_CHECK(false != nvmRes)

	result = memcmp(testData, testDataRead, NVM_BLOCK_2_SIZE);
	printf("\n	* Checking whether read data is the same as the data to be written... ");
	UT_CHECK(0 == result)
	printf("\n");
}

void TestCase3(void)
{
	printf("\n");
	printf("Name: Test case 3\n");
	printf("  Description: Test normal behavior, write all configured blocks\n");
	printf("  Preconditions: The NVManager is initialized\n");
	printf("  Test steps: Write data and then read back all blocks\n");
	printf("  Check results: The read data is the same as the one provided for writing\n");
	printf("  Post steps: none\n");

	uint8_t testDataRead[MAX_DR_SIZE];
	uint32_t ctr=0;
	uint8_t result = 0xFF;
	bool nvmRes = true;

	testDataReadSize = 0;
	nvmRes = true;
	/* check all configured NVM blocks by writing and reading each of them */
	for(ctr=0; ctr<eNvmBlockCount; ctr++)
	{
		memset(testDataRead, 0, NvmBlocks[ctr].size);
		
		fillWithRandom(testData, NvmBlocks[ctr].size);
		nvmRes &= nvm_write(ctr, testData, NvmBlocks[ctr].size);
		nvmRes &= nvm_read(ctr, testDataRead, &testDataReadSize);

		printf("\n	* Checking whether the size of the read data is correct... ");
		UT_CHECK(NvmBlocks[ctr].size == testDataReadSize)
		printf("\n	* Checking whether the NVManager accepted the requests... ");
		UT_CHECK(false != nvmRes)

		result = memcmp(testData, testDataRead, NvmBlocks[ctr].size);
		printf("\n	* Checking whether read data is the same as the data to be written... ");
		UT_CHECK(0 == result)
		printf("\n");
	}
	printf("\n");
}

void TestCase4(void)
{
	printf("\n");
	printf("Name: Test case 4\n");
	printf("  Description: Test exceptional behavior, overflow of the allocated memory are\n");
	printf("  Preconditions: The NVManager is initialized\n");
	printf("  Test steps: Write data a lot of data so that the configured memory area is full. After that test reading and writing\n");
	printf("  Check results: The read data is correct and the write requests are accepted and processed correctly\n");
	printf("  Post steps: none\n");

	uint8_t testDataRead[MAX_DR_SIZE];
	uint32_t ctr=0;
	uint8_t result = 0xFF;
	bool nvmRes = true;

	testDataReadSize = 0;

	/* 1. Write all configured NVM blocks by writing */
	for(ctr=0; ctr<eNvmBlockCount; ctr++)
	{
		nvmRes = true;
		fillWithRandom(testData, NvmBlocks[ctr].size);
		nvmRes &= nvm_write(ctr, testData, NvmBlocks[ctr].size);

		printf("\n	* Checking whether the NVManager accepted the write request... ");
		UT_CHECK(false != nvmRes)
	}
	printf("\n");

	/* 2. Write one of the configured NVM blocks many times until the memory area is full */
	uint32_t configuredFlashAreaSize = NVM_MANAGER_END_ADDR - NVM_MANAGER_START_ADDR;
	uint16_t rawSizeOfTheDR = NvmBlocks[0].size + DR_HEADER_SIZE;
	uint32_t numberOfWriteCyclesRequired = (configuredFlashAreaSize+1)/rawSizeOfTheDR; /* with rounding up */
	nvmRes = true;

	for(ctr=0; ctr<numberOfWriteCyclesRequired; ctr++)
	{
		fillWithRandom(testData, NvmBlocks[0].size);
		nvmRes &= nvm_write(0, testData, NvmBlocks[0].size);
	}
	printf("\n	* Checking whether the NVManager accepted all of the writing requests... ");
	UT_CHECK(false != nvmRes)

	/* 3. Write one block and read it back to verify it is working correctly 
	* 		same as in test case 2 */
	nvmRes = true;
	fillWithRandom(testData, NVM_BLOCK_2_SIZE);
	nvmRes &= nvm_write(eNvmBlock2, testData, NVM_BLOCK_2_SIZE);
	nvmRes &= nvm_read(eNvmBlock2, testDataRead, &testDataReadSize);

	printf("\n	* Checking whether the size of the read data is correct... ");
	UT_CHECK(NVM_BLOCK_2_SIZE == testDataReadSize)
	printf("\n	* Checking whether the NVManager accepted the requests... ");
	UT_CHECK(false != nvmRes)

	result = memcmp(testData, testDataRead, NVM_BLOCK_2_SIZE);
	printf("\n	* Checking whether read data is the same as the data to be written... ");
	UT_CHECK(0 == result)
	printf("\n");

	/* 4. Write all blocks and read data back to verify they are working correctly 
	 * 		same as in Test Case 3 */
	for(ctr=0; ctr<eNvmBlockCount; ctr++)
	{
		memset(testDataRead, 0, NvmBlocks[ctr].size);
		nvmRes = true;
		fillWithRandom(testData, NvmBlocks[ctr].size);
		nvmRes &= nvm_write(ctr, testData, NvmBlocks[ctr].size);
		nvmRes &= nvm_read(ctr, testDataRead, &testDataReadSize);

		printf("\n	* Checking whether the size of the read data is correct... ");
		UT_CHECK(NvmBlocks[ctr].size == testDataReadSize)
		printf("\n	* Checking whether the NVManager accepted the requests... ");
		UT_CHECK(false != nvmRes)

		result = memcmp(testData, testDataRead, NvmBlocks[ctr].size);
		printf("\n	* Checking whether read data is the same as the data to be written... ");
		UT_CHECK(0 == result)
		printf("\n");
	}
	printf("\n");
}

/* main function of the Unit test program */
int main(void)
{
	FILE* fp = NULL;
	int j=0;

	printf("Started execution of the Unit test of the NVManager!\n");

	FlsDrv_Init();
	printf("The flash driver is inialized.\n");

#ifdef LOAD_PREVIOUS_FLASH //  if it is false, it will use always erased flash
	bool bFillData = false;
	// load flash simu from file
	fp = fopen("..\\FlashSimu.bin", "rb");
	if(fp == NULL) { printf("Can't open file to read! Using erased flash\n");	bFillData = true; }

	for(j=0; j<TOTAL_FLASH_SIZE/BUFF_FLASH_PAGE_SIZE; j++)
	{
		fread(FlashSimu[j], 1, BUFF_FLASH_PAGE_SIZE, fp);
	}
	printf("%sThe previous content of the flash is loaded. %s \n", KYEL, KNRM);
	fclose(fp);
#endif

	TestCase1();
	TestCase2();
	TestCase3();
	TestCase4();

	printf("\nUnit test summary:");
	printf("\n%d test cases have been executed.", TestCounter);
	if(TestFailedCounter > 0)
	{
		printf("\n%s %d test cases have failed! %s \n", KRED, TestFailedCounter, KNRM);
	}
	else
	{
		printf("\n%s All test cases have passed! %s \n", KGRN, KNRM);
	}
	printf("%s The execution of the Unit test has ended.\n", KNRM);



	/* save flash simu to file */
	fp = fopen("..\\FlashSimu.bin", "wb");
	if(fp == NULL) { printf("Can't open file to write!\n");	return 1; }

	for(j=0; j<TOTAL_FLASH_SIZE/LOGICAL_PAGE_SIZE; j++)
	{
		fwrite(FlashSimu[j], 1, LOGICAL_PAGE_SIZE, fp);
	}
	fclose(fp);

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile_2.bin"

/* prototypes for test functions */
static void testMultiPageContent(void);

/* main function running all tests */
int main(void) {
	testName = "";

	initStorageManager();

	// test cases added
	testMultiPageContent();

	return 0;
}

/**************************************************************************************
 * Function Name: testMultiPageContent
 *
 * Description:
 *		test multiple pages content
 *
 * Parameters:
 *		NONE
 *
 * Return:
 *		NONE
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------	------------------------
 *		2015-02-16  Jie Zhou <jzhou49@hawk.iit.edu>     Initialization.
 *      2015-02-21  Xin Su <xsu11@hawk.iit.edu>         Change the sequence of the tests,
 *                                                      update 3 to 5 for test of ensureCapacity,
 *                                                      add 3 ops in the test of ensureCapacity,
 *                                                      add detailed comments and logs.
 **************************************************************************************/
void testMultiPageContent(void) {
	SM_FileHandle fh; // file handler
	SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE); // page handler
	int i;

	testName = "test multiple pages content";


	//test createPageFile and openPageFile
	printf("========test createPageFile and openPageFile========\n");

	// create a new page file and create the first empty page
	TEST_CHECK(createPageFile(TESTPF));
	printf("run createPageFile...\n");

	// open the file
	TEST_CHECK(openPageFile(TESTPF, &fh));
	printf("run openPageFile\n");

	ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
	ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");
	printf("created and opened file\n");


	// test readFirstBlock
	printf("========test readFirstBlock========\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	TEST_CHECK(readFirstBlock(&fh, ph));
	printf("run readFirstBlock...\n");

	ASSERT_TRUE((fh.curPagePos == 0), "after readFirstBlock (read first page) page position should be 0");
	printf("read first page\n");

	// the page should be all 0
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
	}
	printf("first block is empty\n");


	// test readCurrentBock
	printf("========test readCurrentBock========\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	TEST_CHECK(readCurrentBlock(&fh, ph));
	printf("run readCurrentBock...\n");

	ASSERT_TRUE((fh.curPagePos == 0), "after readCurrentBlock (read first page) page position should be 0");
	printf("read current (first) page\n");

	// the page should be all zero bytes
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page (readCurrentBlock)");
	}
	printf("current (first) block is empty\n");


	// test appendEmptyBlock
	printf("========test appendEmptyBlock========\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	TEST_CHECK(appendEmptyBlock(&fh));
	printf("run appendEmptyBlock...\n");

	ASSERT_TRUE((fh.totalNumPages == 2), "after appendEmptyBlock (append second page) expect 2 pages");
	ASSERT_TRUE((fh.curPagePos == 1), "after appendEmptyBlock (append second page) page position should be 1");

	readCurrentBlock(&fh, ph);
	printf("read current (second) block\n");

	// the page should be all 0
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in second page (appendEmptyBlock)");
	}
	printf("second block is empty\n");


	// test writeCurrentBlock
	printf("========test writeCurrentBlock========\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	TEST_CHECK(writeCurrentBlock(&fh, ph));
	printf("run writeCurrentBlock with 1...\n");

	ASSERT_TRUE((fh.totalNumPages == 2), "after writeCurrentBlock (write second page) expect 2 pages");
	ASSERT_TRUE((fh.curPagePos == 1), "after writeCurrentBlock (write second page) page position should be 1");

	// set ph to all zero
	memset(ph, 0, PAGE_SIZE);

	readCurrentBlock(&fh, ph);
	printf("read current (second) block\n");

	// this page should be all 1
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == '1'), "expected one byte in second page (writeCurrentBlock)");
	}
	printf("second block is all one\n");

	// backup curPagePos
	int curNum = fh.curPagePos;

	// test error case
	printf("run writeCurrentBlock again when current page position < 0...\n");
	fh.curPagePos = -1;
	ASSERT_ERROR((writeCurrentBlock(&fh, ph)), "writing non-existing page should return an error.");

	// test error case
	printf("run writeCurrentBlock again when current page position > totalNumPages - 1...\n");
	fh.curPagePos = fh.totalNumPages;
	ASSERT_ERROR((writeCurrentBlock(&fh, ph)), "writing non-existing page should return an error.");
    
    // restore curPagePos
    fh.curPagePos = curNum;


	// test readPreviousBlock
	printf("========test readPreviousBlock========\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	TEST_CHECK(readPreviousBlock(&fh, ph));
	printf("run readPreviousBlock...\n");

	ASSERT_TRUE((fh.curPagePos == 0), "after readPreviousBlock (read first page) page position should be 0");
	printf("read previous (first) page\n");

	// the page should be all 0
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page (readPreviousBlock)");
	}
	printf("previous (first) block is empty\n");

	// test error case
	printf("run readPreviousBlock again when current page position is 0...\n");
	ASSERT_ERROR((readPreviousBlock(&fh, ph)), "reading non-existing page should return an error.");


	// test readNextBlock
	printf("========test readNextBlock========\n");

	// set ph to all zero
	memset(ph, 0, PAGE_SIZE);

	TEST_CHECK(readNextBlock(&fh, ph));
	printf("run readNextBlock...\n");

	ASSERT_TRUE((fh.curPagePos == 1), "after readNextBlock (read second page) page position should be 1");
	printf("read next (second) page\n");

	// this page should be all 1
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == '1'), "expected one byte in second page (readNextBlock)");
	}
	printf("next block (second) is all one\n");

	// test error case
	printf("run readNextBlock again when current page position is totalNumPages - 1...\n");
	ASSERT_ERROR((readNextBlock(&fh, ph)), "reading non-existing page should return an error.");


	// test ensureCapacity
	// ensure the total page number is 5
	printf("========test ensureCapacity========\n");
	printf("ensure the total page number is increased to 5\n");

	TEST_CHECK(ensureCapacity(5, &fh));
	printf("run ensureCapacity(5)...\n");

	ASSERT_TRUE((fh.totalNumPages == 5), "after ensureCapacity(5) (add third, fourth and fifth page)expect 5 pages");
	ASSERT_TRUE((fh.curPagePos == 4), "after ensureCapacity(5) (add third, fourth and fifth page) page position should be 4");

	// set ph to all one
	memset(ph, 1, PAGE_SIZE);

	readCurrentBlock(&fh, ph);
	printf("read current (fifth) block\n");

	// this page should be all 0
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in fifth page (ensureCapacity)");
	}
	printf("fifth block is empty\n");

	// change ph to be a string and write that one to disk
	for (i = 0; i < PAGE_SIZE; i++) {
		ph[i] = (i % 10) + '0';
	}

	// write current (fifth) block
	writeCurrentBlock(&fh, ph);
	printf("write current (fifth) block with 0~9 for next test (readLastBlock)\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	// read previous (fourth) block
	readPreviousBlock(&fh, ph);
	printf("read previous (fourth) block\n");

	// this page should be all 0
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in fourth page (ensureCapacity)");
	}
	printf("fourth block is empty\n");

	// set ph to all one
	memset(ph, '1', PAGE_SIZE);

	// read previous (third) block
	readPreviousBlock(&fh, ph);
	printf("read previous (third) block\n");

	// this page should be all 0
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == 0), "expected zero byte in third page (ensureCapacity)");
	}
	printf("third block is empty\n");
    
    // test error case
    printf("run ensureCapacity(4) to check for error case...\n");
    ASSERT_ERROR(ensureCapacity(4, &fh), "no need to add pages as the file has enough pages (ensureCapacity)");
    ASSERT_TRUE((fh.totalNumPages == 5), "after ensureCapacity(4) (no need to add pages) expect 5 pages");


	// test readLastBlock
	// ensure the total page number is 3
	printf("========test readLastBlock========\n");

	TEST_CHECK(readLastBlock(&fh, ph));
	printf("run readLastBlock...\n");

	ASSERT_TRUE((fh.curPagePos == 4), "after readLastBlock (read fifth page) page position should be 4");
	printf("read last (fifth) page\n");

	// this page should be the string (0~9) repeatedly
	for (i = 0; i < PAGE_SIZE; i++) {
		ASSERT_TRUE((ph[i] == (i % 10) + '0'), "expected one to nine byte in last (fifth) page (readLastBlock)");
	}
	printf("last (fifth) block is correct\n");
    
    
    // test readBlock
    printf("========test readBlock========\n");
    
    // set ph to all one
    memset(ph, 0, PAGE_SIZE);
    
    TEST_CHECK(readBlock(1, &fh, ph));
    printf("run readBlock(1)...\n");
    
    ASSERT_TRUE((fh.curPagePos == 1), "after readLastBlock (read fifth page) page position should be 1");
    printf("read last (fifth) page\n");
    
    // this page should be the string (0~9) repeatedly
    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == '1'), "expected one byte in second page (readBlock)");
    }
    printf("second block is correct\n");
    

	// destroy new page file
	TEST_CHECK(destroyPageFile (TESTPF));
	printf("destroy the file\n");

	TEST_DONE()
	;
}

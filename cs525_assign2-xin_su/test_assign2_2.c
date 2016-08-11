#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)			        \
  do {									\
    char *real;								\
    char *_exp = (char *) (expected);                                   \
    real = sprintPoolContent(bm);					\
    if (strcmp((_exp),real) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
	free(real);							\
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
    free(real);								\
  } while(0)

// test and helper methods
static void createDummyPages(BM_BufferPool *bm, int num);

static void testCLOCK(void);

int main(void) {
	initStorageManager();
	testName = "";

	testCLOCK();
}

void createDummyPages(BM_BufferPool *bm, int num) {
	int i;
	BM_PageHandle *h = MAKE_PAGE_HANDLE();

	CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

	for (i = 0; i < num; i++) {
		CHECK(pinPage(bm, h, i));
		sprintf(h->data, "%s-%i", "Page", h->pageNum);
		CHECK(markDirty(bm, h));
		CHECK(unpinPage(bm, h));
	}

	CHECK(shutdownBufferPool(bm));

	free(h);
}

/**************************************************************************************
 * Function Name: testCLOCK
 *
 * Description:
 *		Test case for CLOCK replacement strategy, created based on testFIFO()
 *
 * Parameters:
 *		N/A
 *
 * Return:
 *		void
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
void testCLOCK() {
	// expected results
	const char *poolContents[] = { "[0 0],[-1 0],[-1 0]", // request Page-0
			"[0 0],[1 0],[-1 0]", // request Page-1
			"[0 0],[1 0],[2 0]", // request Page-2
			"[3 0],[1 0],[2 0]", // request Page-3
			"[3 0],[4 0],[2 0]", // request Page-4
			"[3 0],[4 1],[2 0]", // request Page-4
			"[3 0],[4 1],[5x0]", // request Page-5
			"[6x0],[4 1],[5x0]", // request Page-6
			"[6x0],[4 1],[0x0]", // request Page-0
			"[6x0],[4 0],[0x0]", // unpin Page-4
			"[6 0],[4 0],[0 0]" // forceFlushPool()
			};

	const int requests[] = { 0, 1, 2, 3, 4, 4, 5, 6, 0 };
	const int numLinRequests = 5;
	const int numChangeRequests = 3;

	int i;
	BM_BufferPool *bm = MAKE_POOL();
	BM_PageHandle *h = MAKE_PAGE_HANDLE();
	testName = "Testing CLOCK page replacement";

	CHECK(createPageFile("testbuffer.bin"));

	createDummyPages(bm, 100);

	CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_CLOCK, NULL));

	// reading some pages linearly with direct unpin and no modifications
	for (i = 0; i < numLinRequests; i++) {
		pinPage(bm, h, requests[i]);
		unpinPage(bm, h);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}

	// pin one page and test remainder
	i = numLinRequests;
	pinPage(bm, h, requests[i]);
	ASSERT_EQUALS_POOL(poolContents[i], bm, "pool content after pin page");

	// read pages and mark them as dirty
	for (i = numLinRequests + 1; i < numLinRequests + numChangeRequests + 1;
			i++) {
		pinPage(bm, h, requests[i]);
		markDirty(bm, h);
		unpinPage(bm, h);
		ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
	}

	// flush buffer pool to disk
	i = numLinRequests + numChangeRequests + 1;
	h->pageNum = 4;
	unpinPage(bm, h);
	ASSERT_EQUALS_POOL(poolContents[i], bm, "unpin last page");

	i++;
	forceFlushPool(bm);
	ASSERT_EQUALS_POOL(poolContents[i], bm, "pool content after flush");

	// check number of write IOs
	ASSERT_EQUALS_INT(3, getNumWriteIO(bm), "check number of write I/Os");
	ASSERT_EQUALS_INT(8, getNumReadIO(bm), "check number of read I/Os");

	CHECK(shutdownBufferPool(bm));
	CHECK(destroyPageFile("testbuffer.bin"));

	free(bm);
	free(h);
	TEST_DONE()
	;
}

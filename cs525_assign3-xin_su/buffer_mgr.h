#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"

// Include bool DT
#include "dt.h"

// Replacement Strategies
typedef enum ReplacementStrategy {
	RS_FIFO = 0, RS_LRU = 1, RS_CLOCK = 2, RS_LFU = 3, RS_LRU_K = 4
} ReplacementStrategy;

/* Data Types and Structures */
typedef int PageNumber;
#define NO_PAGE -1

// Buffer Manager Buffer Pool Handler
typedef struct BM_BufferPool {
	char *pageFile;
	int numPages;
	ReplacementStrategy strategy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
					// manager needs for a buffer pool
} BM_BufferPool;

// Buffer Manager Page Handler
typedef struct BM_PageHandle {
	PageNumber pageNum;
	char *data;
} BM_PageHandle;

/* BEGIN 2015-03-03, added by Xin Su <xsu11@hawk.iit.edu> */
/* Page Frame Structure:
 * 		struct BM_pageHandle *page: the Page Handler in Buffer Manager that stores page content and page number
 * 		int frameNum: frame number
 * 		int numReadIO: the number of the read IO on this page
 * 		int numWriteIO: the number of the write IO on this page
 * 		int fixCount: When a page is in using, the fixCount is 0
 * 		bool dirtyFlag: When a page is modified (content updated or deleted), the dirtyFlag is set to TRUE
 * 		struct PageFrame *previous: the pointer that is pointing to the previous PageFrame
 * 		struct PageFrame *next: the pointer that is pointing to the next PageFrame
 * */
typedef struct PageFrame {
	struct BM_PageHandle *page;
	int frameNum;
	int numReadIO;
	int numWriteIO;
	int fixCount;
	bool dirtyFlag;
	bool clockFlag;
	struct PageFrame *previous;
	struct PageFrame *next;
} PageFrame;

/* Page List Structure
 * 		struct PageFrame *head: the pointer that is pointing to the head element of the List
 * 		struct PageFrame *tail: the pointer that is pointing to the tail element of the List
 * 		struct PageFrame *current: the pointer that is pointing to the current element of the List
 * 		struct PageFrame *clock: the pointer used for CLOCK replacement strategy
 * 		int size: the actual size of the List
 * */
typedef struct PageList {
	struct PageFrame *head;
	struct PageFrame *tail;
	struct PageFrame *current;
	struct PageFrame *clock;
	int size;
} PageList;
/* END 2015-03-03, added by Xin Su <xsu11@hawk.iit.edu> */

// convenience macros
#define MAKE_POOL()					\
  ((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()				\
  ((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool * const bm, const char * const pageFileName,
		const int numPages, ReplacementStrategy strategy, void *stratData);
RC shutdownBufferPool(BM_BufferPool * const bm);
RC forceFlushPool(BM_BufferPool * const bm);

// Buffer Manager Interface Access Pages
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page);
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page);
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page);
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents(BM_BufferPool * const bm);
bool *getDirtyFlags(BM_BufferPool * const bm);
int *getFixCounts(BM_BufferPool * const bm);
int getNumReadIO(BM_BufferPool * const bm);
int getNumWriteIO(BM_BufferPool * const bm);

#endif

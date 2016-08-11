#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "dt.h"

/* GLOBAL VARIABLES */
SM_FileHandle *F_HANDLE; // file handler to store file's content and info
PageNumber *PAGE_NUMS; // array of pageNum
bool *DIRTY_FLAGS; // array of dirtyFlag
int *FIX_COUNTS; // array of fixCount
int NUM_READ_IOS; // number of read page since the BUffer Pool initializes
int NUM_WRITE_IOS; // number of write page since the BUffer Pool initializes
pthread_rwlock_t rwlock; // read-write lock

/**************************************************************************************
 * Function Name: searchPage
 *
 * Description:
 *		Search the requested page in the Buffer Pool,
 *		if found, load the requested page into the BM_pageHandle and return RC_OK
 *		else, return error code
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC searchPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		PageNumber pageNum) {
	PageList *queue = (PageList *) bm->mgmtData;

	// if the size of the PageList < 0, then the PageList is not initialized, return RC_PAGELIST_NOT_INITIALIZED
	// else if the size of the PageList = 0, then the PageList is empty, return RC_PAGE_NOT_FOUND
	if (queue->size < 0) {
		return RC_PAGELIST_NOT_INITIALIZED;
	} else if (queue->size == 0) {
		return RC_PAGE_NOT_FOUND;
	}

	/*
	 * If the code comes here, then the PageList is initialized and not empty
	 * The size of the PageList > 0
	 * Search the requested page starting from the head
	 */

	// Set the current pointer to the head of the PageList
	queue->current = queue->head;

	while (queue->current != queue->tail
			&& queue->current->page->pageNum != pageNum) {
		queue->current = queue->current->next;
	}

	// After the while loop, if the current comes to the tail,
	// then We still need to determine if the tail contains the requested page
	if (queue->current == queue->tail
			&& queue->current->page->pageNum != pageNum) {
		return RC_PAGE_NOT_FOUND;
	}

	/*
	 * If the code comes here, then the requested page is found in the PageList
	 * Set statistics and load the requested page into BM_pageHandle
	 */

	// Load the content into BM_PageHandle
	page->data = queue->current->page->data;
	page->pageNum = queue->current->page->pageNum;

	// Return RC_PAGE_FOUND instead of RC_OK to let this function can be used both in FIFO and LRO
	return RC_PAGE_FOUND;
} // searchPage

/**************************************************************************************
 * Function Name: appendPage
 *
 * Description:
 *		Read the requested page from the disk and append it to the tail of the PageList
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>			Add thread read-write lock
 **************************************************************************************/
RC appendPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		PageNumber pageNum) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init return code

	// Require lock
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);

	// Open File
	rc = -99;
	rc = openPageFile(bm->pageFile, F_HANDLE);

	if (rc != RC_OK) {
		return rc;
	}

	// if the size of the PageList = 0, then the PageList is empty, add this requested page as the head
	// else, the PageList is neither empty nor full, add the requested page to next of the tail of the PageList
	if (queue->size == 0) {
		queue->head->fixCount = 1;
		queue->head->numWriteIO = 1;

		// if the page does not exist, then call ensureCapacity to add the requested page to the file
		if (F_HANDLE->totalNumPages < pageNum + 1) {
			int totalPages = F_HANDLE->totalNumPages;
			rc = -99;
			rc = ensureCapacity(pageNum + 1, F_HANDLE);
			NUM_WRITE_IOS += pageNum + 1 - totalPages;

			if (rc != RC_OK) {
				// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

				// Close file
				rc = -99;
				rc = closePageFile(F_HANDLE);

				if (rc != RC_OK) {
					return rc;
				}

				// Release lock
				pthread_rwlock_unlock(&rwlock);
				pthread_rwlock_destroy(&rwlock);

				return rc;
			}
		}
		queue->head->numWriteIO = 0;

		// After ensureCapacity, now we can read the requested page from the file
		queue->head->numReadIO++;
		rc = -99;
		rc = readBlock(pageNum, F_HANDLE, queue->head->page->data);
		NUM_READ_IOS++;
		queue->head->numReadIO--;

		if (rc != RC_OK) {
			// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

			// Close file
			rc = -99;
			rc = closePageFile(F_HANDLE);

			if (rc != RC_OK) {
				return rc;
			}

			// Release lock
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);

			return rc;
		}

		// Now the fixCount = 1, the numReadIO = 0, and the numWriteIO = 0
		queue->head->page->pageNum = pageNum;
		queue->head->dirtyFlag = FALSE;
		queue->head->clockFlag = FALSE;

		// Now there is only 1 page in the PageList, and all pointers , including the current pointer, are pointing to it
	} else {
		queue->tail->next->fixCount = 1;
		queue->tail->next->numWriteIO = 1;

		// if the page does not exist, then call ensureCapacity to add the requested page to the file
		if (F_HANDLE->totalNumPages < pageNum + 1) {
			int totalPages = F_HANDLE->totalNumPages;
			rc = -99;
			rc = ensureCapacity(pageNum + 1, F_HANDLE);
			NUM_WRITE_IOS += pageNum + 1 - totalPages;

			if (rc != RC_OK) {
				// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

				// Close file
				rc = -99;
				rc = closePageFile(F_HANDLE);

				if (rc != RC_OK) {
					return rc;
				}

				// Release lock
				pthread_rwlock_unlock(&rwlock);
				pthread_rwlock_destroy(&rwlock);

				return rc;
			}
		}
		queue->tail->next->numWriteIO = 0;

		// After ensureCapacity, now we can read the requested page from the file
		queue->tail->next->numReadIO++;
		rc = -99;
		rc = readBlock(pageNum, F_HANDLE, queue->tail->next->page->data);
		NUM_READ_IOS++;
		queue->tail->next->numReadIO--;

		if (rc != RC_OK) {
			// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

			// Close file
			rc = -99;
			rc = closePageFile(F_HANDLE);

			if (rc != RC_OK) {
				return rc;
			}

			// Release lock
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);

			return rc;
		}

		// Now the fixCount = 1, the numReadIO = 0, and the numWriteIO = 0
		queue->tail->next->page->pageNum = pageNum;
		queue->tail->next->dirtyFlag = FALSE;
		queue->tail->next->clockFlag = FALSE;

		queue->tail = queue->tail->next;

		// Set the current pointer to the requested page, that is the tail of the PageList
		queue->current = queue->tail;
	}

	// After appending the requested page, Increment the size of the PageList
	queue->size++;

	// Load the requested page into BM_PageHandle
	page->data = queue->current->page->data;
	page->pageNum = queue->current->page->pageNum;

	// Close file
	rc = -99;
	rc = closePageFile(F_HANDLE);

	if (rc != RC_OK) {
		return rc;
	}

	// Release lock
	pthread_rwlock_unlock(&rwlock);
	pthread_rwlock_destroy(&rwlock);

	return RC_OK;
} // appendPage

/**************************************************************************************
 * Function Name: replacePage
 *
 * Description:
 *		Replace the current page with the requested page read from the disk
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>			Add thread read-write lock
 **************************************************************************************/
RC replacePage(BM_BufferPool * const bm, BM_PageHandle * const page,
		PageNumber pageNum) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init return code

	// Require lock
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);

	// Open file
	rc = -99;
	rc = openPageFile(bm->pageFile, F_HANDLE);

	if (rc != RC_OK) {
		return rc;
	}

	// If the removable page is dirty, then write it back to the disk before remove it.
	// Now the fixCount = 0, and the numReadIO = 0 and the numWriteIO = 0
	queue->current->fixCount = 1;
	queue->current->numWriteIO = 1;

	// if the removable page is dirty, then write it back to the file
	if (queue->current->dirtyFlag == TRUE) {
		rc = -99;
		rc = writeBlock(queue->current->page->pageNum, F_HANDLE,
				queue->current->page->data);
		NUM_WRITE_IOS++;

		if (rc != RC_OK) {
			// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

			// Close file
			rc = -99;
			rc = closePageFile(F_HANDLE);

			if (rc != RC_OK) {
				return rc;
			}

			// Release unlock
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);

			return rc;
		}

		// After writeBlock, set the PageFrame back to clean
		queue->current->dirtyFlag = FALSE;
	}

	// if the page does not exist, then call ensureCapacity to add the requested page to the file
	if (F_HANDLE->totalNumPages < pageNum + 1) {
		int totalPages = F_HANDLE->totalNumPages;
		rc = -99;
		rc = ensureCapacity(pageNum + 1, F_HANDLE);
		NUM_WRITE_IOS += pageNum + 1 - totalPages;

		if (rc != RC_OK) {
			// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

			// Close file
			rc = -99;
			rc = closePageFile(F_HANDLE);

			if (rc != RC_OK) {
				return rc;
			}

			// Release unlock
			pthread_rwlock_unlock(&rwlock);
			pthread_rwlock_destroy(&rwlock);

			return rc;
		}
	}
	queue->current->numWriteIO = 0;

	// After ensureCapacity, now we can read the requested page from the file
	queue->current->numReadIO++;
	rc = -99;
	rc = readBlock(pageNum, F_HANDLE, queue->current->page->data);
	NUM_READ_IOS++;
	queue->current->numReadIO--;

	if (rc != RC_OK) {
		// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

		// Close file
		rc = -99;
		rc = closePageFile(F_HANDLE);

		if (rc != RC_OK) {
			return rc;
		}

		// Release lock
		pthread_rwlock_unlock(&rwlock);
		pthread_rwlock_destroy(&rwlock);

		return rc;
	}

	// Load the requested page to the current PageFrame in the BM_BufferPool
	// Now the fixCount = 1, the numReadIO = 0, and the numWriteIO = 0
	queue->current->page->pageNum = pageNum;
	queue->current->clockFlag = FALSE;

	// Load the requested into BM_PageHandle
	page->data = queue->current->page->data;
	page->pageNum = queue->current->page->pageNum;

	// Close file
	rc = -99;
	rc = closePageFile(F_HANDLE);

	if (rc != RC_OK) {
		return rc;
	}

	// Release lock
	pthread_rwlock_unlock(&rwlock);
	pthread_rwlock_destroy(&rwlock);

	return RC_OK;
} // replacePage

/**************************************************************************************
 * Function Name: FIFO
 *
 * Description:
 *		FIFO replacement strategy
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization.
 **************************************************************************************/
RC FIFO(BM_BufferPool * const bm, BM_PageHandle * const page,
		PageNumber pageNum) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init return code

	// First search the requested page in the PageList
	printf("searchPage: Page-%d\n", pageNum);
	rc = -99;
	rc = searchPage(bm, page, pageNum);

	// First search the requested page in the queue
	// if the requested page is found, then set statistics and return rc (RC_PAGE_FOUND)
	// else if something unexpected error happens, then return rc
	if (rc == RC_PAGE_FOUND) {
		queue->current->fixCount++;
		printf("Page-%d found\n", pageNum);

		return rc;
	} else if (rc != RC_PAGE_NOT_FOUND) {
		return rc;
	}
	printf("Page-%d not found\n", pageNum);

	/*
	 * If the code comes here, then the Buffer Manager doesn't have the requested page
	 * We have to read the page from the disk and load it into BM_PageHandle.
	 */

	// if the Buffer Manager has vacancy for the requested page,
	// then read the page from the disk and append it to the next of the tail of the PageList
	if (queue->size < bm->numPages) {
		printf("appendPage: Page-%d\n", pageNum);
		rc = -99; // reset return code
		rc = appendPage(bm, page, pageNum);

		if (rc == RC_OK) {
			printf("Page-%d appended\n", pageNum);
		}

		return rc;
	}

	/*
	 * If the code comes here, then neither the Buffer Manager has the requested page loaded nor vacancy for the requested page
	 * Now the PageList is full: the size of the PageList = bm->numPages
	 * We have to replace an existing page in the Buffer Manager with the requested page
	 */

	// Find the first removable page, starting from head
	// Set the current pointer to the head of the queue to start the traversal
	queue->current = queue->head;

	// Find the first page with fixCount = 0
	while (queue->current != queue->tail
			&& (queue->current->fixCount != 0 || queue->current->numReadIO != 0
					|| queue->current->numWriteIO != 0)) {
		queue->current = queue->current->next;
	}

	// After the while loop, if the current pointer comes to the tail,
	// then we still need to determine if the tail's fixCount = 0
	if (queue->current == queue->tail
			&& (queue->current->fixCount != 0 || queue->current->numReadIO != 0
					|| queue->current->numWriteIO != 0)) {
		return RC_NO_REMOVABLE_PAGE;
	}

	/*
	 * If the code comes here, then a removable page is found
	 * The current pointer is pointing to it
	 */

	// Replace the removable page with the requested page
	printf("replacePage: Page-%d\n", pageNum);
	rc = -99; // reset the return value
	rc = replacePage(bm, page, pageNum);

	// if replacePage completes without error and the requested page is not in the tail spot
	// then move it to the tail of the PageList
	if (rc == RC_OK && queue->current != queue->tail) {
		// Remove the current PageFrame
		// The steps of the remove are different depending on if the requested page is the head or not
		if (queue->current == queue->head) {
			queue->head = queue->head->next;
			queue->current->next->previous = NULL;
		} else {
			queue->current->previous->next = queue->current->next;
			queue->current->next->previous = queue->current->previous;
		}

		// Add the requested page to the tail of the PageList
		// step 1 - connect tail and current
		queue->current->previous = queue->tail;
		queue->tail->next = queue->current;

		// step 2 - set current's next
		queue->current->next = NULL;

		// step 3 - set tail
		queue->tail = queue->tail->next;

		printf("Page-%d replaced\n", pageNum);

		// Now the current pointer still points to the requested page
	}

	return rc;
} // FIFO

/**************************************************************************************
 * Function Name: LRU
 *
 * Description:
 *		LRU replacement strategy
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC LRU(BM_BufferPool * const bm, BM_PageHandle * const page, PageNumber pageNum) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init return code

	// Run FIFO first
	rc = -99;
	rc = FIFO(bm, page, pageNum);

	// if FIFO meets error, then return the error code
	// else if return RC_PAGE_FOUND, then FIFO completes with searchPage,
	// also if the requested page is not in the tail spot, move it to the tail of the PageList
	if (rc != RC_OK && rc != RC_PAGE_FOUND) {
		return rc;
	} else if (rc == RC_PAGE_FOUND && queue->current != queue->tail) {
		// Now the current pointer points to the requested page
		// All we need to do is move the requested page to the tail of the PageList

		// Remove the current PageFrame
		// The steps of the remove are different depending on if the requested page is the head or not
		if (queue->current == queue->head) {
			queue->head = queue->head->next;
			queue->current->next->previous = NULL;
		} else {
			queue->current->previous->next = queue->current->next;
			queue->current->next->previous = queue->current->previous;
		}

		// Add the current PageFrame to the tail
		// step 1 - connect tail and current
		queue->current->previous = queue->tail;
		queue->tail->next = queue->current;

		// step 2 - set current's next
		// Be careful that maybe the PageList is not full
		// if the PageList is not full, then we should also set the previous pointer of the next PageFrame to the tail to the requested page
		if (queue->size < bm->numPages) {
			queue->current->next = queue->tail->next;
			queue->tail->next->previous = queue->current;
		} else {
			queue->current->next = NULL;
		}

		// step 3 - set tail
		queue->tail = queue->tail->next;

		// Now the current pointer still points to the requested page
	}

	/*
	 * If the code comes here, then LRU complete
	 * Now the current pointer points to the requested page
	 */

	return RC_OK;
} // LRU

/**************************************************************************************
 * Function Name: CLOCK
 *
 * Description:
 *		CLOCK replacement strategy
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC CLOCK(BM_BufferPool * const bm, BM_PageHandle * const page,
		PageNumber pageNum) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init return code

	// First search the page in the PageList
	printf("searchPage: Page-%d\n", pageNum);
	rc = -99;
	rc = searchPage(bm, page, pageNum);

	// if find the page, set the current page's clockFlag to TRUE
	// else if other errors happen, return rc
	if (rc == RC_PAGE_FOUND) {
		queue->current->fixCount++;
		queue->current->clockFlag = TRUE;
		printf("Page-%d found\n", pageNum);

		return rc;
	} else if (rc != RC_PAGE_NOT_FOUND) {
		return rc;
	}
	printf("Page-%d not found\n", pageNum);

	/*
	 * If the code comes here, then the Buffer Manager doesn't have the page loaded.
	 * We have to read the page from the disk and load it into BM_PageHandle.
	 */

	// if the Buffer Manager has vacancy for the requested page,
	// then read the page from the disk and append it to the next to the tail of the PageList
	if (queue->size < bm->numPages) {
		printf("appendPage: Page-%d\n", pageNum);
		rc = -99; // reset return code
		rc = appendPage(bm, page, pageNum);

		// if the PageList is not full, set the clock pointer to the next to the current pointer
		// else, now the current pointer points to the tail, then set the clock pointer points back to the head
		if (rc == RC_OK) {
			if (queue->size < bm->numPages) {
				queue->clock = queue->current->next;
			} else if (queue->size == bm->numPages) {
				queue->clock = queue->head;
			}

			printf("Page-%d appended\n", pageNum);
		}

		return rc;
	}

	/*
	 * If the code comes here, then neither the Buffer Manager has the page loaded nor vacancy for the requested page
	 * Now the PageList is full: the size of the PageList = bm->numPages
	 * We have to replace an existing page in the Buffer Manager with the requested page
	 */

	// Logic of CLOCK replacement strategy
	// Now the current pointer points to the page that was requested last time,
	// and the clock pointer points to the next to the current pointer (or back to the head if the current pointers points to the tail)
	// Find the first page with clockFlag = TRUE and fixCount = 0
	while (queue->clock->clockFlag == TRUE || queue->clock->fixCount != 0
			|| queue->clock->numReadIO != 0 || queue->clock->numWriteIO != 0) {
		queue->clock->clockFlag = FALSE;

		// Into the while loop means this page does not fit the requirement of the removable page
		// Move the clock pointer to the next. If it points to the tail, move it back to the head
		if (queue->clock == queue->tail) {
			queue->clock = queue->head;
		} else {
			queue->clock = queue->clock->next;
		}
	}

	// We find the first PageFrame whose clockFlag = FALSE and fixCount = 0
	// Set the current pointer to the clock pointer, so that we can call replacePage
	queue->current = queue->clock;

	// Replace the removable page with the requested page
	printf("replacePage: Page-%d\n", pageNum);
	rc = -99;
	rc = replacePage(bm, page, pageNum);

	if (rc == RC_OK) {
		// After the replacement of the requested page, set its clockFlag to TRUE
		queue->clock->clockFlag = TRUE;

		// Set the clock pointer to the next to the current pointer
		if (queue->clock == queue->tail) {
			queue->clock = queue->head;
		} else {
			queue->clock = queue->clock->next;
		}

		printf("Page-%d replaced\n", pageNum);
	}

	return rc;
} // CLOCK

/**************************************************************************************
 * Function Name: initPageList
 *
 * Description:
 *		Initialize the PageList to store pages in the Buffer Pool
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *
 * Return:
 *		void
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-03-15  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
void initPageList(BM_BufferPool * const bm) {
	PageList *queue = (PageList *) bm->mgmtData;
	PageFrame *pf[bm->numPages];

	int i;
	for (i = 0; i < bm->numPages; i++) {
		// init PageFrame -> position 3
		// Free them in shutdownBufferPool -> position 2
		pf[i] = (PageFrame *) malloc(sizeof(PageFrame));

		// Initialize the content in the PageFrame
		pf[i]->page = MAKE_PAGE_HANDLE();

		// init page->data -> position 4
		// Free them in shutdownBufferPool -> position 1
		pf[i]->page->data = (char *) malloc(PAGE_SIZE * sizeof(char));
		pf[i]->page->pageNum = NO_PAGE;
		pf[i]->frameNum = i;
		pf[i]->numReadIO = 0;
		pf[i]->numWriteIO = 0;
		pf[i]->fixCount = 0;
		pf[i]->dirtyFlag = FALSE;
		pf[i]->clockFlag = FALSE;

		// Add this new PageFrame to the tail of pf

		// if pf has only one node (i = 0), then add this new PageFrame as the head
		// else, add this new PageFrame to the tail
		if (i == 0) {
			pf[i]->previous = NULL;
			pf[i]->next = NULL;
		} else {
			pf[i - 1]->next = pf[i];
			pf[i]->previous = pf[i - 1];
			pf[i]->next = NULL;
		}
	}

	// Reset all pointers and queue's size to the initial state
	queue->head = pf[0];
	queue->tail = queue->head;
	queue->current = queue->head;
	queue->clock = queue->head;
	queue->size = 0;

	return;
} // initPageList

// Buffer Manager Interface Pool Handling

/**************************************************************************************
 * Function Name: initBufferPool
 *
 * Description:
 *		Initialize the Buffer Pool
 *
 * Parameters:
 *		BM_BufferPool *const bm: the Buffer Pool Handler that the user wants to initiate
 *		const char * const pageFileName: the name of the requested page file
 *		const int numPages: capacity of the Buffer Pool
 *		ReplacementStrategy strategy; replacement strategy
 *		void *stratData: N/A
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-03-14  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Delete the unused queue
 *															Add comments
 *															Add the judgment of the rc returned by openPageFile
 **************************************************************************************/
RC initBufferPool(BM_BufferPool * const bm, const char * const pageFileName,
		const int numPages, ReplacementStrategy strategy, void *stratData) {
	// Make sure the capacity of the Buffer Pool is valid
	if (numPages <= 0) {
		return RC_INVALID_NUMPAGES;
	}

	// init F_HANDLE, PAGE_NUMS, DIRTY_FLAGS, FIX_COUNTS -> position 1
	// Free them in shutdownBufferPool -> position 4
	F_HANDLE = (SM_FileHandle *) malloc(sizeof(SM_FileHandle));
	PAGE_NUMS = (PageNumber *) malloc(bm->numPages * sizeof(PageNumber)); // the pointer to store the entry of the array and use for return
	DIRTY_FLAGS = (bool *) malloc(bm->numPages * sizeof(bool)); // the pointer to store the entry of the array and use for return
	FIX_COUNTS = (int *) malloc(bm->numPages * sizeof(int)); // the pointer to store the entry of the array and use for return

	// init NUM_READ_IOS, NUM_WRITE_IOS
	NUM_READ_IOS = 0;
	NUM_WRITE_IOS = 0;

	// init BM_BufferPool's profiles
	bm->pageFile = (char *) pageFileName; // set the name of the requested page file
	bm->numPages = numPages; // set the capacity of the Buffer Pool
	bm->strategy = strategy; // set the replacement strategy

	// init PageList and store the entry to bm->mgmtData -> position 2
	// Free it in shutdownBufferPool -> position 3
	PageList *queue = (PageList *) malloc(sizeof(PageList));
	bm->mgmtData = queue;

	// init the PageList and store the entry in bm->mgmtData
	initPageList(bm);

	return RC_OK;
} // initBufferPool

/**************************************************************************************
 * Function Name: shutdownBufferPool
 *
 * Description:
 *		Shut down the Buffer Pool
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-03-14  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Add judgment of the rc returned by forceFlushPool
 *															Modify the logic of freeing the PageList
 *															Add comments
 *															Add judgment of the rc returned by closePageFile
 *															Add the free code of F_HANDLE, PAGE_NUMS, DIRTY_FLAGS, FIX_COUNTS
 **************************************************************************************/
RC shutdownBufferPool(BM_BufferPool * const bm) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init return code

	// Flush all dirty pages in the Buffer Pool back to the disk
	rc = -99;
	rc = forceFlushPool(bm);

	if (rc != RC_OK) {
		return rc;
	}

	/*
	 * If the code comes here, then all pages in the Buffer Poll are clean
	 * Now we need to free allocated memory blocks (PageFrame) of the PageList inside out
	 */

	// Set the current pointer to the tail
	queue->current = queue->tail;

	// if the capacity is 1, simply free the only block
	// else (the capacity > 1), free the PageFrame from the tail back to the head
	if (bm->numPages == 1) {
		// Free page->data -> position 1
		free(queue->head->page->data);

		// Free PageFrame -> position 2
		free(queue->head->page);
	} else {
		while (queue->current != queue->head) {
			// First set the current pointer to the previous of itself so that the while loop can move on
			queue->current = queue->current->previous;

			// Free page->data -> position 1
			free(queue->current->next->page->data);

			// Free PageFrame -> position 2
			free(queue->current->next->page);
		}

		// After the while loop, the current pointer points to the head, then free the only left block
		// Free page->data of the head -> position 1
		free(queue->head->page->data);

		// Free PageFrame of the head -> position 2
		free(queue->head->page);
	}

	// Free queue -> position 3
	free(queue);

	// Then free the F_HANDLE, PAGE_NUMS, DIRTY_FLAGS, FIX_COUNTS -> position 4
	free(F_HANDLE);
	free(PAGE_NUMS);
	free(DIRTY_FLAGS);
	free(FIX_COUNTS);

	return RC_OK;
} // shutdownBufferPool

/**************************************************************************************
 * Function Name: forceFlushPool
 *
 * Description:
 *		Write back the data in all dirty pages in the Buffer Pool
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-03-14  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Modify the PageList queue
 *															Add comments
 *															Add judgment of the rc returned by write function
 *		2015-03-27	Xin Su <xsu11@hawk.iit.edu>				Add validation of whether there is any unwritable page
 **************************************************************************************/
RC forceFlushPool(BM_BufferPool * const bm) {
	PageList *queue = (PageList *) bm->mgmtData;
	int unwritableCount = 0;

	// Set the current pointer to the head
	queue->current = queue->head;

	// Before writing a page back to disk, do check whether the fixCount = 0
	// Just call forcePage() to write a dirty and writable page back to the file

	// The while loop ends when the current pointer points to the tail
	while (queue->current != queue->tail) {
		// Validate whether the page is writable
		// if the page is unwritable, then increment the unwritableCount
		// else if the page is dirty and writable, call forcePage() to write the page back to the file
		if (queue->current->dirtyFlag == TRUE && queue->current->fixCount > 0) {
			unwritableCount++;
		} else if (queue->current->dirtyFlag == TRUE
				&& queue->current->fixCount == 0) {
			forcePage(bm, queue->current->page);
		}

		queue->current = queue->current->next;
	}

	// After the while loop, the current points to the tail of the PageList
	// We still need to check the page in the tail, using the same logic above
	if (queue->current == queue->tail) {
		if (queue->current->dirtyFlag == TRUE && queue->current->fixCount > 0) {
			unwritableCount++;
		} else if (queue->current->dirtyFlag == TRUE
				&& queue->current->fixCount == 0) {
			forcePage(bm, queue->current->page);
		}
	}

	// if there is any unwritable page, then return error code
	if (unwritableCount != 0) {
		return RC_FLUSH_POOL_ERROR;
	}

	return RC_OK;
} // forceFlushPool

// Buffer Manager Interface Access Pages

/**************************************************************************************
 * Function Name: pinPage
 *
 * Description:
 *		Pin the page with the requested pageNum in the BUffer Pool
 *		If the page is not in the Buffer Pool, load it from the file to the Buffer Pool
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *		PageNumber pageNum: the page number of the requested page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-03-17  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>			Modify the logic of pinning the requested page
 *														Add comments
 **************************************************************************************/
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {
	RC rc = -99; // init the return code

	if (bm->strategy == RS_FIFO) {
		rc = FIFO(bm, page, pageNum);

		// Because the searchPage() in the FIFO() returns a different return code (RC_PAGE_FOUND)
		// when it completes without errors from the return code (RC_OK) returned by appendPage() and replacePage(),
		// so it should be reset to RC_OK when searchPage() is executed and returned successfully
		if (rc == RC_PAGE_FOUND) {
			rc = RC_OK;
		}
	} else if (bm->strategy == RS_LRU) {
		rc = LRU(bm, page, pageNum);
	} else if (bm->strategy == RS_CLOCK) {
		rc = CLOCK(bm, page, pageNum);
	} else if (bm->strategy == RS_LFU) {
		// Replacement strategy is not implemented yet
		return RC_RS_NOT_IMPLEMENTED;
	} else if (bm->strategy == RS_LRU_K) {
		// Replacement strategy is not implemented yet
		return RC_RS_NOT_IMPLEMENTED;
	}

	return rc;
} // pinPage

/**************************************************************************************
 * Function Name: markDirty
 *
 * Description:
 *		Mark the requested page as dirty
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-03-12  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>			Modify the logic of searching the requested page
 *														Add comments
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>			Add thread read-write lock
 **************************************************************************************/
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc; // init the return code

	// First search the requested page in the PageList
	rc = -99;
	rc = searchPage(bm, page, page->pageNum);

	// if something unexpected error happens or page not found, then return rc
	if (rc != RC_PAGE_FOUND) {
		return rc;
	}

	// Now the current pointer points to the requested page
	// Mark the requested page (pointed by the current pointer) dirty
	queue->current->dirtyFlag = TRUE;

	return RC_OK;
} // markDirty

/**************************************************************************************
 * Function Name: unpinPage
 *
 * Description:
 *		Unpin a page
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-03-12  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>			Modify the logic of unpinning the requested page
 *														Add comments
 **************************************************************************************/
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc;

	// First search the requested page in the queue
	rc = -99;
	rc = searchPage(bm, page, page->pageNum);

	// if something unexpected error happens  or page not found, then return rc
	if (rc != RC_PAGE_FOUND) {
		return rc;
	}

	// Set statistics
	queue->current->fixCount--;
	return RC_OK;
} // unpinPage

/**************************************************************************************
 * Function Name: forcePage
 *
 * Description:
 *		Write the requested page back to the page file on disk
 *
 * Parameters:
 *		BM_BufferPool * const bm: Buffer Pool Handler
 *		BM_PageHandle * const page: Buffer Page Handler
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-03-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>			Modify the logic of forcing to write the requested page back
 *														Add comments
 **************************************************************************************/
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {
	PageList *queue = (PageList *) bm->mgmtData;
	RC rc = -99;

	// Require lock
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);

	// Open file
	rc = -99;
	rc = openPageFile(bm->pageFile, F_HANDLE);

	if (rc != RC_OK) {
		return rc;
	}

	// First set numWriteIO
	queue->current->numWriteIO = 1;

	// Write the requested page back to the disk
	rc = -99;
	rc = writeBlock(page->pageNum, F_HANDLE, page->data);
	NUM_WRITE_IOS++;

	if (rc != RC_OK) {
		// Do not change fixCount and NumWriteIO back, for this indicates write IO error and need more info to proceed

		// Close file
		rc = -99;
		rc = closePageFile(F_HANDLE);

		if (rc != RC_OK) {
			return rc;
		}

		// Release unlock
		pthread_rwlock_unlock(&rwlock);
		pthread_rwlock_destroy(&rwlock);

		return rc;
	}

	// Set the page back to clean
	queue->current->dirtyFlag = FALSE;

	// The write completes without error, then set numWriteIO back
	queue->current->numWriteIO = 0;

	// Close file
	rc = -99;
	rc = closePageFile(F_HANDLE);

	if (rc != RC_OK) {
		return rc;
	}

	// Release lock
	pthread_rwlock_unlock(&rwlock);
	pthread_rwlock_destroy(&rwlock);

	return RC_OK;
} // forcePage

// Statistics Interface

/**************************************************************************************
 * Function Name: getFrameContents
 *
 * Description:
 *		 Returns an array of PageNumbers (of size numPages)
 *		 An empty page frame is represented using the constant NO_PAGE
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		PageNumber *: an array of PageNumber of size numPages
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  --------------------------------------	------------------------
 *		2015-03-11  Xiaolang Wang <xwang122@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Optimize the code in format
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>				Change local array variable to global
 **************************************************************************************/
PageNumber *getFrameContents(BM_BufferPool * const bm) {
	PageList *queue = (PageList *) bm->mgmtData;

	// Set the current pointer to head
	queue->current = queue->head;

	int pos = 0; // Let pos be the position in the array, initially set to 0

	while (queue->current != queue->tail) {
		// Put the value of the page pointed to by the current pointer into the current position of array
		PAGE_NUMS[pos] = queue->current->page->pageNum;

		queue->current = queue->current->next;
		pos++;
	}

	// Now the current pointer points to the tail
	// Include the tail's info into the array, then increment the pos to set it be equal to the size of the PageList
	PAGE_NUMS[pos++] = queue->current->page->pageNum;

	// Now pos = the size of the PageList
	// if pos < numPages, then the PageList is not full, add the rest values of the BUffer Pool into the array
	if (pos < bm->numPages) {
		int i;
		for (i = pos; i < bm->numPages; i++) {
			PAGE_NUMS[i] = NO_PAGE;
		}
	}

	queue->current = queue->tail;

	return PAGE_NUMS;
} // getFrameContents

/**************************************************************************************
 * Function Name: getDirtyFlags
 *
 * Description:
 *		 Returns an array of bools (of size numPages)
 *		 Empty page frames are considered as clean
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		bool *: an array of bools indicating each page is dirty or not
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  --------------------------------------  ------------------------
 *		2015-03-11  Xiaolang Wang <xwang122@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Optimize the code in format
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>				Change local array variable to global
 **************************************************************************************/
bool *getDirtyFlags(BM_BufferPool * const bm) {
	PageList *queue = (PageList *) bm->mgmtData;

	queue->current = queue->head; //set current to head

	int pos = 0; // let pos be the position in array, initially it's 0
	while (queue->current != queue->tail) {
		// Put the value of the page pointed to by the current pointer into the current position of array
		DIRTY_FLAGS[pos] = queue->current->dirtyFlag;

		queue->current = queue->current->next;
		pos++; // pos moves to next position
	}

	// Now the current pointer points to the tail
	// Include the tail's info into the array, then increment the pos to set it be equal to the size of the PageList
	DIRTY_FLAGS[pos++] = queue->current->dirtyFlag;

	// Now pos = the size of the PageList
	// if pos < numPages, then the PageList is not full, add the rest values of the BUffer Pool into the array
	if (pos < bm->numPages) {
		int i;
		for (i = pos; i < bm->numPages; i++) {
			DIRTY_FLAGS[i] = FALSE;
		}
	}

	queue->current = queue->tail;

	return DIRTY_FLAGS;
} // getDirtyFlags

/**************************************************************************************
 * Function Name: getFixCounts
 *
 * Description:
 *		 Returns an array of ints (of size numPages)
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		int *: an array of fix count of each page in the BufferPool
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  --------------------------------------  ------------------------
 *		2015-03-11  Xiaolang Wang <xwang122@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Optimize the code in format
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>				Change local array variable to global
 **************************************************************************************/
int *getFixCounts(BM_BufferPool * const bm) {
	PageList *queue = (PageList *) bm->mgmtData;

	queue->current = queue->head; //set current to head

	int pos = 0; // let pos be the position in array, initially it's 0
	while (queue->current != queue->tail) {
		// Put the value of the page pointed to by the current pointer into the current position of array
		FIX_COUNTS[pos] = queue->current->fixCount;

		queue->current = queue->current->next;
		pos++; // pos moves to next position
	}

	// Now the current pointer points to the tail
	// Include the tail's info into the array, then increment the pos to set it be equal to the size of the PageList
	FIX_COUNTS[pos++] = queue->current->fixCount;

	// Now pos = the size of the PageList
	// if pos < numPages, then the PageList is not full, add the rest values of the BUffer Pool into the array
	if (pos < bm->numPages) {
		int i;
		for (i = pos; i < bm->numPages; i++) {
			FIX_COUNTS[i] = 0;
		}
	}

	queue->current = queue->tail;

	return FIX_COUNTS;
} // getFixCounts

/**************************************************************************************
 * Function Name: getNumReadIO
 *
 * Description:
 *		Returns the number of pages that have been read from disk
 *		since the Buffer Pool has been initialized
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		int: the number of pages that have been read from disk
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  --------------------------------------  ------------------------
 *		2015-03-11  Xiaolang Wang <xwang122@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Optimize the code in format
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>				Change local array variable to global
 **************************************************************************************/
int getNumReadIO(BM_BufferPool * const bm) {
	return NUM_READ_IOS;
} // getNumReadIO

/**************************************************************************************
 * Function Name: getNumWriteIO
 *
 * Description:
 *		Return the number of pages written to the page file
 *		since the Buffer Pool has been initialized
 *
 * Parameters:
 *		BM_BufferPool *const bm: Buffer Pool Handler
 *
 * Return:
 *		int: the number of pages written to the page file
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  --------------------------------------  ------------------------
 *		2015-03-11  Xiaolang Wang <xwang122@hawk.iit.edu>	Initialization
 *		2015-03-20	Xin Su <xsu11@hawk.iit.edu>				Optimize the code in format
 *		2015-03-26	Xin Su <xsu11@hawk.iit.edu>				Change local array variable to global
 **************************************************************************************/
int getNumWriteIO(BM_BufferPool * const bm) {
	return NUM_WRITE_IOS;
} // getNumWriteIO

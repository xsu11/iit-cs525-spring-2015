#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#include "dberror.h"

/************************************************************
 *                    handle data structures                *
 ************************************************************/
typedef struct SM_FileHandle {
	char *fileName;     // file name
	int totalNumPages;  // total number of the pages in this file
	int curPagePos;     // current page position
	void *mgmtInfo;     // store additional information - file pointer
} SM_FileHandle; // file handler to store file's content and info

typedef char* SM_PageHandle; // a pointer to an area in memory storing the data of a page

/************************************************************
 *                    interface                             *
 ************************************************************/
/* manipulating page files */
extern void initStorageManager(void);
extern RC createPageFile(char *fileName);
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle);
extern RC closePageFile(SM_FileHandle *fHandle);
extern RC destroyPageFile(char *fileName);

/* reading blocks from disc */
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern int getBlockPos(SM_FileHandle *fHandle);
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* writing blocks to a page file */
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC appendEmptyBlock(SM_FileHandle *fHandle);
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle);

#endif

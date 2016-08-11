#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dberror.h"
#include "storage_mgr.h"

/* totalNumPages is the total page number of the file. When there is no page, it is 0.
 * curPos is the current position of the file. It starts from 0.
 * pageNum is the pageNum-th page of the file. It starts from 0. */

/* ====================================jzhou49==================================== */
/**************************************************************************************
 * Function Name: initStorageManager
 *
 * Description:
 *		Initialize the StorageManager.
 *
 * Parameters:
 *		None
 *
 * Return:
 *		None
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-02-07  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization.
 *		2015-02-15  Jie Zhou <jzhou49@hawk.iit.edu>		change the format of comment.
 **************************************************************************************/
void initStorageManager(void) {
	// initialize the storageManager
	printf("Initializing StorageManager...\n");
}

/**************************************************************************************
 * Function Name: createPageFile
 *
 * Description:
 *		Create a page file and write content.
 *
 * Parameters:
 *		char *fileName: file name 
 *
 * Return:
 *		RC: returned code
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------	------------------------
 *		2015-02-07  Jie Zhou <jzhou49@hawk.iit.edu>     Initialization.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>			Add free memory and close file code.
 *		2015-02-15  Jie Zhou <jzhou49@hawk.iit.edu>		Add header comment,
 *                                                      add comments.
 **************************************************************************************/
RC createPageFile(char *fileName) {
	RC rc = -99; // return value.

	// set up a file pointer fp, open a file with mode "wb+".
	// "wb+" Create an empty file and open it for update (both for input and output), 
	// If a file with the same name already exists its content is erased and the file is considered as a new empty file.
	FILE *fp = fopen(fileName, "wb+");

	// check the fp, if the fp is NULL, return msg
	if (fp == NULL) {
		return RC_FILE_NOT_FOUND;
	}

	// Allocates a block of memory for an array of PAGE_SIZE elements,
	// initializes all its bits to zero.
	// emptyPage stores the memory and content allocated.
	SM_PageHandle emptyPage = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));

	// write an empty page into the file
	int writeStatus = fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);

	if (writeStatus == 0) {
		rc = RC_WRITE_FILE_FAILED;
	} else {
		rc = RC_OK;
	}

	// free heap memory
	free(emptyPage);

	// close file
	int closeLabel = fclose(fp);

	if (closeLabel == EOF) {
		rc = RC_CLOSE_FILE_FAILED;
	}

	return rc;
}

/**************************************************************************************
 * Function Name: openPageFile
 *
 * Description:
 *		Open the created pageFile and store relative information into fHandle.
 *
 * Parameters:
 *		char *fileName: file name 
 *		SM_FileHandle *fHandle: file handler
 *
 * Return:
 *		RC: returned value
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>n
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-02-07  Jie Zhou <jzhou49@hawk.iit.edu>     Initialization.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>			Change code as the total number start from 1 and
 *														pageNum and curPagePos start from 0.
 *		2015-02-15  Jie Zhou <jzhou49@hawk.iit.edu>		Add header comment.
 **************************************************************************************/
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
	// Open a file for update both reading and writing. The file must exist. 
	FILE *fp = fopen(fileName, "r+");

	// check status open file
	if (fp == NULL) {
		return RC_FILE_NOT_FOUND;
	}

	// calculate the file's total page number
	// move position pointer to the end of the file
	int seekLabel = fseek(fp, 0L, SEEK_END);

	if (seekLabel != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	// get the last position of the file
	long tailPos = ftell(fp);

	if (tailPos == -1L) {
		return RC_SEEK_FILE_TAIL_ERROR;
	}

	// get file length
	int fileLength = (int) tailPos + 1;
	int totalNum = fileLength / PAGE_SIZE;

	// move position pointer to the beginning of the file
	seekLabel = fseek(fp, 0L, SEEK_SET);

	if (seekLabel != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	// initialize the file's information into fHandle.
	fHandle->fileName = fileName;
	fHandle->totalNumPages = totalNum;
	fHandle->curPagePos = 0;
	fHandle->mgmtInfo = fp;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: closePageFile
 *
 * Description:
 *		Close the file.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file handler
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-02-07  Jie Zhou <jzhou49@hawk.iit.edu>     Initialization.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>			Add validation of closing file.
 *		2015-02-15  Jie Zhou <jzhou49@hawk.iit.edu>		Add header comment.
 **************************************************************************************/
RC closePageFile(SM_FileHandle *fHandle) {
	// close the file and capture the status.
	int closeLabel = fclose(fHandle->mgmtInfo);

	// check the return value of fclose operation.
	if (closeLabel == EOF) {
		return RC_CLOSE_FILE_FAILED;
	}

	return RC_OK;
}

/* ====================================xwang122==================================== */
/**************************************************************************************
 * Function Name: destroyPageFile
 *
 * Description:
 *		Destroy (delete) a page file.
 *
 * Parameters:
 *		char *fileName: the name of the file that is to delete
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-02-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-02-10  Xiaolang Wang <xwang122@hawk.iit.edu>     Correct the remove() function call,
 *                                                            add comments.
 **************************************************************************************/
RC destroyPageFile(char *fileName) {
	// use isRemoved to check if the file is deleted
	int isRemoved = remove(fileName);

	// if isRemoved == 0, the file remove successfully, return RC_OK
	// if not, remove is not successed, return RC_FILE_NOT_FOUND
	if (isRemoved != 0) {
		return RC_REMOVE_FILE_FAILED;
	}

	return RC_OK;
}

/**************************************************************************************
 * Function Name: readBlock
 *
 * Description:
 *		The method reads the pageNum-th block from a file and
 *		stores its content in the memory pointed to by the memPage page handle.
 *		If the file has less than pageNum pages, the method should return RC_READ_NON_EXISTING_PAGE.
 *
 * Parameters:
 *		int pageNum: the number of the page that is about to read
 *		SM_FileHandle *fHandle: the file handler that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-02-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-02-10  Xiaolang Wang <xwang122@hawk.iit.edu>     Correct the all incorrect *fHandle to fHandle,
 *															  add comments, correct totalPage to totalNumPage,
 *															  add check length of the read result == PAGE_SIZE.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>				  Change code as the total number start from 1 and
 *															  pageNum and curPagePos start from 0.
 **************************************************************************************/
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// check if the page number is valid
	// invalid page number includes pageNum + 1 > totalNumPage and pageNum < 0
	// if invalid, return RC_READ_NON_EXISTING_PAGE
	if (fHandle->totalNumPages < (pageNum + 1) || pageNum < 0) {
		return RC_NON_EXISTING_PAGE;
	}

	// calculate the offset from the starting point
	int offset = sizeof(char) * pageNum * PAGE_SIZE;

	// use isSet to check if the pointer is successfully set
	// if isSet != 0, return RC_SET_POINTER_FAILED
	int isSet = fseek(fHandle->mgmtInfo, offset, SEEK_SET);

	if (isSet != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	// isRead is used to check if read successfully
	// if isRead != PAGE_SIZE, the read is not successful, return RC_READ_FAILED
	// else return RC_OK
	int isRead = fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);

	if (isRead != PAGE_SIZE) {
		return RC_READ_FILE_FAILED;
	}

	// set current page to the entered pageNum
	fHandle->curPagePos = pageNum;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: getBlockPos
 *
 * Description:
 *		Return the current page position in a file.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *
 * Return:
 *		int: current page number
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-02-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-02-10  Xiaolang Wang <xwang122@hawk.iit.edu>     Correct the all incorrect *fHandle to fHandle,
 *															  add comments.
 **************************************************************************************/
int getBlockPos(SM_FileHandle *fHandle) {
	// curPagePos in the structure is to store the current page position, simply return it
	return fHandle->curPagePos;
}

/**************************************************************************************
 * Function Name: readFirstBlock
 *
 * Description:
 *		Return the first page position of a file.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		int: current page number
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-02-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-02-10  Xiaolang Wang <xwang122@hawk.iit.edu>     Correct the all incorrect *fHandle to fHandle,
 *															  add comments.
 **************************************************************************************/
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// call readBlock with pageNum = 0
	return readBlock(0, fHandle, memPage);
}

/* ====================================czhao18==================================== */
/**************************************************************************************
 * Function Name: readPreviousBlock
 *
 * Description:
 *		Read page from the previous block.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  ---------------------------------------	------------------------
 *		2015-02-10  Chengnan Zhao <czhao18@hawk.iit.edu>	Add method description.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>				Change code as the total number start from 1 and
 *															pageNum and curPagePos start from 0.
 **************************************************************************************/
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
	int numPage = fHandle->curPagePos - 1;
	return readBlock(numPage, fHandle, memPage);
}

/**************************************************************************************
 * Function Name: readCurrentBlock
 *
 * Description:
 *		Read page from the Current block.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------  ------------------------
 *		2015-02-10  Chengnan Zhao <czhao18@hawk.iit.edu>	Add methods description.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>				Change code as the total number start from 1 and
 *															pageNum and curPagePos start from 0.
 **************************************************************************************/
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
	int numPage = fHandle->curPagePos;
	return readBlock(numPage, fHandle, memPage);
}

/**************************************************************************************
 * Function Name: readNextBlock
 *
 * Description:
 *		Read one page from the Next block.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------  ------------------------
 *		2015-02-10  Chengnan Zhao <czhao18@hawk.iit.edu>	Add methods description.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>				Change code as the total number start from 1 and
 *															pageNum and curPagePos start from 0. **************************************************************************************/
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
	int numPage = fHandle->curPagePos + 1;
	return readBlock(numPage, fHandle, memPage);
}

/**************************************************************************************
 * Function Name: readLastBlock
 *
 * Description:
 *		Read page from the last block.
 *
 * Parameters:
 *		int numPage: stores the page number which is last page number
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------  ------------------------
 *		2015-02-10  Chengnan Zhao <czhao18@hawk.iit.edu>	Add methods description.
 *		2015-02-14	Xin Su <xsu11@hawk.iit.edu>				Change code as the total number start from 1 and
 *															pageNum and curPagePos start from 0.
 **************************************************************************************/
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
	int numPage = fHandle->totalNumPages - 1;
	return readBlock(numPage, fHandle, memPage);
}

/* ====================================xsu11==================================== */
/**************************************************************************************
 * Function Name: writeBlock
 *
 * Description:
 *		Write one page BACK to the file (disk) start from absolute position.
 *
 * Parameters:
 *		int pageNum: the start point of the page number in the file
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------  ------------------------
 *		2015-02-09  Xin Su <xsu11@hawk.iit.edu>			Initialization.
 *		2015-02-14  Xin Su <xsu11@hawk.iit.edu>			Change code as the total number start from 1 and
 *														pageNum and curPagePos start from 0.
 **************************************************************************************/
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// If pageNum < 0 or file's totalNumPages <= pageNum,
	// then the page cannot be written to the file.
	// Return RC_WRITE_FAILED.
	if (pageNum < 0 || fHandle->totalNumPages < (pageNum + 1)) {
		return RC_NON_EXISTING_PAGE;
	}

	// Get the offset and seek the position in the file.
	// If the position that supposes to be the start point is not found,
	// then the page cannot be written to the file.
	// Return RC_SEEK_FILE_POSITION_ERROR
	int offset = pageNum * PAGE_SIZE * sizeof(char); // offset in the file from the absolute position

	int seekLabel = fseek(fHandle->mgmtInfo, offset, SEEK_SET); // return label

	if (seekLabel != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	// If the writing operation does not complete,
	// then the page is not written to the file completely.
	// Return RC_WRITE_FAILED
	// TODO - what if only part of the stream is written to the file?
	// TODO - whether it needs to backup before write the block over the original file?
	int writtenSize = fwrite(memPage, sizeof(char), PAGE_SIZE,
			fHandle->mgmtInfo); // return size

	if (writtenSize != PAGE_SIZE) {
		return RC_WRITE_FILE_FAILED;
	}

	// set current position to PageNum
	fHandle->curPagePos = pageNum;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: writeCurrentBlock
 *
 * Description:
 *		Write one page BACK to the file (disk) of current position.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
 *		SM_PageHandle memPage: the page handler that points to the stream to be written to the file
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------  ------------------------
 *		2015-02-09  Xin Su <xsu11@hawk.iit.edu>			Initialization.
 **************************************************************************************/
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
	RC rc = writeBlock(fHandle->curPagePos, fHandle, memPage); // return code
	return rc;
}

/**************************************************************************************
 * Function Name: appendEmptyBlock
 *
 * Description:
 *		Write an empty page to the file (disk) by appending to the end.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: the file handle that contains the file info
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
 *		2015-02-09  Xin Su <xsu11@hawk.iit.edu>			Initialization.
 *		2015-02-15	Xin Su <xsu11@hawk.iit.edu>			Delete fclose before return.
 *      2015-02-17  Jie Zhou <jzhou49@hawk.iit.edu>     Add fHandle->curPagePos.
 **************************************************************************************/
RC appendEmptyBlock(SM_FileHandle *fHandle) {
	RC rc = -99; // return code, initialized as -99

	// file pointer to the file being handled
	FILE *fp = fHandle->mgmtInfo;

	if (fp == NULL) {
		return RC_FILE_NOT_FOUND;
	}

	// create an empty page
	SM_PageHandle emptyPage = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));

	// move position pointer to the end of the file
	int seekLabel = fseek(fp, 0L, SEEK_END);

	if (seekLabel != 0) {
		return RC_SEEK_FILE_POSITION_ERROR;
	}

	// write into the file, return the written size
	// TODO - what if only part of the stream is written to the file?
	int writtenSize = fwrite(emptyPage, sizeof(char), PAGE_SIZE,
			fHandle->mgmtInfo);

	if (writtenSize != PAGE_SIZE) {
		rc = RC_WRITE_FILE_FAILED;
	} else {
		fHandle->curPagePos = fHandle->totalNumPages++;
		rc = RC_OK;
	}

	// free heap memory
	free(emptyPage);

	return rc;
}

/**************************************************************************************
 * Function Name: ensureCapacity
 *
 * Description:
 *		If the file has less than numberOfPages pages then increase the size to numberOfPages.
 *
 * Parameters:
 *		int numberOfPages: the number of the pages that the file needs to be increased to
 *		SM_FileHandle *fHandle: the file handle that contains the file info
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
 *		2015-02-09  Xin Su <xsu11@hawk.iit.edu>         Initialization.
 *		2015-02-15	Xin Su <xsu11@hawk.iit.edu>			Add validation for appendEmptyBlock inside the for loop.
 *      2015-02-15	Xin Su <xsu11@hawk.iit.edu>			Store totalNumPages to a local variable before the for loop
 **************************************************************************************/
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
	if (fHandle->totalNumPages >= numberOfPages) {
		return RC_ENOUGH_PAGES;
	}
    
    // store totalNumPages to a local variable
    int totalNum = fHandle->totalNumPages;
    int i;
    
	for (i = 0; i < (numberOfPages - totalNum); i++) {
		RC rc = appendEmptyBlock(fHandle);

        // TODO - what should I do to clean the unfinished write of the empty block?
		if (rc != RC_OK) {
			return rc;
		}
	}
	return RC_OK;
}

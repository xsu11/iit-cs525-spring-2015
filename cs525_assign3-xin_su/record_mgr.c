#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "dt.h"
#include "expr.h"
#include "record_mgr.h"
#include "storage_mgr.h"
#include "tables.h"

#define TABLE_INFO_PAGE 0
#define FIRST_RESERVED_PAGE 1

#define RESERVED_PAGE_HEADER 96
#define RESERVED_PAGE_CAPACITY \
	sizeof(char) * 8 * (PAGE_SIZE - RESERVED_PAGE_HEADER)

BM_BufferPool *BM;

Schema *SCHEMA = NULL;
char *SCHEMA_STR = NULL;

int RECORD_LENGTH = -1;
int PAGE_MAX_RECORDS = -1;

// RID *R_ID = NULL;

RM_TableData *REL = NULL;

// prototypes
RC initRecordManager(void *mgmtData);
RC shutdownRecordManager();
RC initTableInfoPage();
RC createTable(char *name, Schema *schema);
RC openTable(RM_TableData *rel, char *name);
RC closeTable(RM_TableData *rel);
RC deleteTable(char *name);
int getRecordLength(RM_TableData *rel);
int getTotalRecords(RM_TableData *rel);
RC setTotalRecords(RM_TableData *rel, int value);
int getTotalPages(RM_TableData *rel);
RC setTotalPages(RM_TableData *rel, int value);
int getPageMaxRecords(RM_TableData *rel);
int getNumTuples(RM_TableData *rel);
RC checkRID(RM_TableData *rel, RID id);
PageNumber getReservedPageNum(PageNumber pageNum);
bool isPageFree(RM_TableData *rel, PageNumber reservedPageNum,
		PageNumber pageNum);
bool getBit(char *data, int bitOffset);
RC setBit(char *data, int bitOffset, bool value);
RC createReservedPage(RM_TableData *rel, PageNumber reservedPageNum);
RC updateReservedPage(RM_TableData *rel, PageNumber reservedPageNum,
		PageNumber pageNum, bool value);
PageNumber getFirstFreePage(RM_TableData *rel, PageNumber reservedPageNum);
RC setFirstFreePage(RM_TableData *rel, PageNumber reservedPageNum, int value);
PageNumber getNextReservedPage(RM_TableData *rel, PageNumber reservedPageNum);
RC setNextReservedPage(RM_TableData *rel, PageNumber reservedPageNum, int value);
PageNumber getPrevReservedPage(RM_TableData *rel, PageNumber reservedPageNum);
RC setPrevReservedPage(RM_TableData *rel, PageNumber reservedPageNum, int value);
PageNumber searchFirstFreePage(RM_TableData *rel);
bool isPageInitialized(RM_TableData *rel, PageNumber pageNum);
RC setPageInitialized(RM_TableData *rel, PageNumber pageNum, bool value);
PageNumber getCurrentRecords(RM_TableData *rel, PageNumber pageNum);
RC setCurrentRecords(RM_TableData *rel, PageNumber pageNum, int value);
bool isSlotOccupied(RM_TableData *rel, PageNumber pageNum, int slotId);
RC setSlotOccupied(RM_TableData *rel, PageNumber pageNum, int slotId,
		bool value);
RC checkRecordLength(RM_TableData *rel, char *recordData);
RC initRecordPage(RM_TableData *rel, PageNumber pageNum);
int getFreeSlotId(RM_TableData *rel, PageNumber pageNum);
RC insertRecord(RM_TableData *rel, Record *record);
RC deleteRecord(RM_TableData *rel, RID id);
RC updateRecord(RM_TableData *rel, Record *record);
RC getRecord(RM_TableData *rel, RID id, Record *record);
RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond);
RC next(RM_ScanHandle *scan, Record *record);
RC closeScan(RM_ScanHandle *scan);
int getRecordSize(Schema *schema);
char *schemaToString(Schema * schema);
Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes,
		int *typeLength, int keySize, int *keys);
RC freeSchema(Schema *schema);
RC createRecord(Record **record, Schema *schema);
RC freeRecord(Record *record);
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value);
RC setAttr(Record *record, Schema *schema, int attrNum, Value *value);

/**************************************************************************************
 * Function Name: initRecordManager
 *
 * Description:
 *		Initialize the Record Manager
 *
 * Parameters:
 *		void *mgmtData: not defined
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC initRecordManager(void *mgmtData) {
	printf("BEGIN-initRecordManager\n");
	printf("Initializing Record Manager...\n");
	RC rc;

	/*
	 * I have to do this to meet assign3's test cases, even it is complete wrong
	 * The entire init process should be done together in a startup script
	 */

	// init BUffer Manager
	// init parameters for Buffer Manager
	if (BM == NULL) {
		BM = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
	}

	ReplacementStrategy strategy = RS_LRU;
	int numPages = 10;

	// Call initBufferPool() to init Buffer Manager (10, LRU)
	// Shutdown it in shutdownRrecordManager()
	rc = -99;
	rc = initBufferPool(BM, "", numPages, strategy, NULL);

	if (rc != RC_OK) {
		return rc;
	}

	/*
	 * Back to normal...
	 */

	if (SCHEMA == NULL) {
		SCHEMA = (Schema *) malloc(sizeof(Schema));

		SCHEMA->numAttr = -1;
		SCHEMA->attrNames = NULL;
		SCHEMA->dataTypes = NULL;
		SCHEMA->typeLength = NULL;
		SCHEMA->keyAttrs = NULL;
		SCHEMA->keySize = -1;
	}

	if (SCHEMA_STR == NULL) {
		SCHEMA_STR = (char *) calloc(PAGE_SIZE, sizeof(char));
	}

	/*
	 if (R_ID == NULL) {
	 R_ID = (RID *) malloc(sizeof(RID));

	 R_ID->page = FIRST_RESERVED_PAGE + 1;
	 R_ID->slot = -1;
	 }
	 */

	if (REL == NULL) {
		REL = (RM_TableData *) malloc(sizeof(RM_TableData));

		REL->name = NULL;
		REL->schema = SCHEMA;
		REL->mgmtData = BM;
	}

	printf("END\n\n");
	return RC_OK;
} // initRecordManager

/**************************************************************************************
 * Function Name: shutdownRecordManager
 *
 * Description:
 *		Shutdown the Record Manager
 *
 * Parameters:
 *		N/A
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC shutdownRecordManager() {
	printf("BEGIN-shutdownRecordManager\n");
	RC rc;

	// free(R_ID);
	// R_ID = NULL;

	free(SCHEMA_STR);
	SCHEMA_STR = NULL;

	free(SCHEMA);
	SCHEMA = NULL;

	/*
	 * Clean the mess left by initRecordManager()...
	 */

	// Shutdown Buffer Manager
	rc = -99;
	rc = shutdownBufferPool(BM);

	if (rc != RC_OK) {
		return rc;
	}

	free(BM);
	BM = NULL;

	free(REL);
	REL = NULL;

	printf("END\n\n");
	return RC_OK;
} // shutdownRecordManager

/**************************************************************************************
 * Function Name: initTableInfoPage
 *
 * Description:
 *		init the table info page
 *
 * Parameters:
 *		N/A
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
 *		2015-04-08  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
RC initTableInfoPage() {
	printf("BEGIN-initTableInfoPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// Pin the page that stores the table info, which is page TABLE_INFO_PAGE (0)
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// Set totalRecords and totalPages to 0
	int totalRecords = 0;
	int totalPages = 1;

	// Write the following info into the table info page:
	// RECORD_LENGTH: int
	// totalRecords: int
	// totalPages: int
	// PAGE_MAX_RECORDS: int
	// SCHEMA_STR: char * (seperated by '|')
	int *ip = (int *) page->data; // create a pointer with type int * to store int data
	ip[0] = RECORD_LENGTH;
	ip[1] = totalRecords;
	ip[2] = totalPages;
	ip[3] = PAGE_MAX_RECORDS;

	char *cp = (char *) (ip + 4); // Create a pointer with char * to store string data
	memcpy(cp, SCHEMA_STR, strlen(SCHEMA_STR));

	// Unpin the table info page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // initTableInfoPage

/**************************************************************************************
 * Function Name: createTable
 *
 * Description:
 *		Create table file if it doesn't exist
 *		Then init the table info page and create the first reserved page
 *
 * Parameters:
 *		Char *name: file name and table name
 *		Schema *schema: table schema
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
 *		2015-04-08  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-04-13	Xin Su <xsu11@hawk.iit.edu>				Add comments
 *															Write sample code for different storing of the data into the memory block
 **************************************************************************************/
RC createTable(char *name, Schema *schema) {
	printf("BEGIN-createTable\n");
	RC rc; // return code

	// if the transformed SCHEMA_STR is empty, return an error code
	if (SCHEMA == NULL || strlen(SCHEMA_STR) == 0) {
		return RC_SCHEMA_NOT_CREATED;
	}

	// Check if the file exists
	rc = -99;
	rc = access(name, F_OK);

	// if the file does not exist (rc == -1), create it by calling createPageFile(),
	// else if the file exists (rc == 0), then return error code to report it
	// else, some error happens, return rc
	if (rc == -1) {
		rc = -99;
		rc = createPageFile(name);

		if (rc != RC_OK) {
			return rc;
		}
	} else if (rc == 0) {
		return RC_TABLE_EXISTS;
	} else {
		return rc;
	}

	// Set the name of the file to the Buffer Manager
	BM->pageFile = name;

	// Now init the table info page
	rc = -99;
	rc = initTableInfoPage();

	if (rc != RC_OK) {
		return rc;
	}

	// Then create the first reserved page, which is page FIRST_RESERVED_PAGE (1)
	rc = -99;
	rc = createReservedPage(NULL, FIRST_RESERVED_PAGE);

	printf("END\n\n");
	return rc;
} // createTable

/**************************************************************************************
 * Function Name: openTable
 *
 * Description:
 *		Open table
 *
 * Parameters:
 * 		RM_TableData *rel: input table
 *		char *name: table name
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
 *		2015-04-09  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-04-13	Xin Su <xsu11@hawk.iit.edu>				Add comments
 *															Add check on the data to be set
 **************************************************************************************/
RC openTable(RM_TableData *rel, char *name) {
	printf("BEGIN-openTable\n");
	// We don't need to init Buffer Manager in this function
	// It was inited in initRecordManager
	// Therefore we only set all the info as needed

	/*
	 // Check if the global variable SCHEMA_STR is empty
	 // TODO - do we need to check the content inside the SCHEMA?
	 if (strlen(name) == 0) {
	 return RC_OPEN_TABLE_FAILED;
	 }

	 // TODO - do we need to check of the content inside the rel->schema is NULL?
	 if (rel->schema == NULL && SCHEMA_STR != NULL) {
	 rel->schema = SCHEMA;
	 }
	 */

	// if the file has no access, then return error code to report it
	if (access(name, R_OK) != 0 || access(name, W_OK) != 0) {
		return RC_TABLE_FILE_NO_ACCESS;
	}

	// Set the name of the file to the Buffer Manager
	BM->pageFile = name;

	// Set the table structure
	rel->name = name;
	rel->schema = SCHEMA;

	// Store the entry of the Buffer Manager here in the table structure
	rel->mgmtData = BM;

	printf("END\n\n");
	return RC_OK;
} // openTable

/**************************************************************************************
 * Function Name: closeTable
 *
 * Description:
 *		Close table
 *
 * Parameters:
 * 		RM_TableData *rel: input table
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
 *		2015-04-09  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-04-13	Xin Su <xsu11@hawk.iit.edu>				Add comments
 **************************************************************************************/
RC closeTable(RM_TableData *rel) {
	printf("BEGIN-closeTable\n");
	// BM_BufferPool *BM = (BM_BufferPool *) rel->mgmtData;
	RC rc;

	// Write all updated content in the Buffer Pool
	rc = -99;
	rc = forceFlushPool(BM);

	if (rc != RC_OK) {
		return rc;
	}
	// Unset the file name in Buffer Manager
	BM->pageFile = NULL;

	// Unset the table info
	rel->name = NULL;
	rel->schema = NULL;
	rel->mgmtData = NULL;

	printf("END\n\n");
	return RC_OK;
} // closeTable

/**************************************************************************************
 * Function Name: deleteTable
 *
 * Description:
 *		Get isPageInitialized (TRUE/FALSE)
 *
 * Parameters:
 *		char *name: table name
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC deleteTable(char *name) {
	printf("BEGIN-deleteTable\n");
	RC rc;

	rc = destroyPageFile(name);

	if (rc != RC_OK) {
		return rc;
	}

	printf("END\n\n");
	return RC_OK;
} // deleteTable

/**************************************************************************************
 * Function Name: getRecordLength
 *
 * Description:
 *		return the single record length stored in info page
 *
 * Parameters:
 * 		RM_TableData *rel: input table
 *
 * Return:
 *		int
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-09  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-04-12  Chengnan Zhao <czhao18@hawk.iit.edu>	Revise
 **************************************************************************************/
int getRecordLength(RM_TableData *rel) {
	printf("BEGIN-getRecordLength\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	// change data pointer into integer pointer
	int *ip = (int *) page->data; // pointer to int

	// first integer is the target length, store the value in temporary variable
	int recordLength = ip[0];

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return recordLength;
} // getRecordLength

/**************************************************************************************
 * Function Name: getTotalRecords
 *
 * Description:
 *		Get the total records number stored in the table info page
 *
 * Parameters:
 * 		RM_TableData *rel: input table
 *
 * Return:
 *		int
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-09  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 *		2015-04-09  Xin Su <xsu11@hawk.iit.edu>				Use different method to retrieve the data in the memory block
 **************************************************************************************/
int getTotalRecords(RM_TableData *rel) {
	printf("BEGIN-getTotalRecords\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data; // pointer to int
	int totalRecords = ip[1];

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return totalRecords;
} // getTotalRecords

/**************************************************************************************
 * Function Name: setTotalRecords
 *
 * Description:
 *		Set the total records number to user's input number
 *
 * Parameters:
 * 		RM_TableData *rel: input table
 * 		int recordNum: user input number
 *
 * Return:
 *		int
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-13  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
RC setTotalRecords(RM_TableData *rel, int value) {
	printf("BEGIN-setTotalRecords\n");

	if (value < 0) {
		return RC_SET_TOTAL_RECORDS_ERROR;
	}

	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;

	ip[1] = value;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setTotalRecords

/**************************************************************************************
 * Function Name: getTotalPages
 *
 * Description:
 *		Get the total page number stored in info page
 *
 * Parameters:
 * 		RM_TableData *rel: input table
 *
 * Return:
 *		int
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-13  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
int getTotalPages(RM_TableData *rel) {
	printf("BEGIN-getTotalPages\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data; // pointer to int
	int totalPages = ip[2];

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return totalPages;
} // getTotalPages

/**************************************************************************************
 * Function Name: setTotalPages
 *
 * Description:
 *		Set the total pages number to user's input number
 *
 * Parameters:
 * 		char *data: input table data
 * 		int value: user input number
 *
 * Return:
 *		int
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-13  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
RC setTotalPages(RM_TableData *rel, int value) {
	printf("BEGIN-setTotalPages\n");

	if (value < 1) {
		return RC_SET_TOTAL_PAGES_ERROR;
	}

	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;
	ip[2] = value;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setTotalPages

/**************************************************************************************
 * Function Name: getPageMaxRecords
 *
 * Description:
 *		return the max records number that can store in one page
 *
 * Parameters:
 * 		RM_TableData *rel: input table
 *
 * Return:
 *		int: integer format
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-13  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
int getPageMaxRecords(RM_TableData *rel) {
	printf("BEGIN-getPageMaxRecords\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, TABLE_INFO_PAGE);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data; // pointer to int
	int pageMaxRecords = ip[3];

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return pageMaxRecords;
} // getPageMaxRecords

/**************************************************************************************
 * Function Name: getNumTuples
 *
 * Description:
 *		Get the total number of records stored in table
 *
 * Parameters:
 * 		RM_TableData *rel: input table data
 *
 * Return:
 *		int
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-09  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
int getNumTuples(RM_TableData *rel) {
	printf("BEGIN-getNumTuples\n");

	// call method which returns current number of records store in table
	RC rc = getTotalRecords(rel);

	printf("END\n\n");
	return rc;
} // getNumTuples

/**************************************************************************************
 * Function Name: checkRID
 *
 * Description:
 *		check is the passed in RID is valid
 *
 * Parameters:
 * 		RM_TableData *rel: input table data
 * 		RID id: this RID we want to check
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
 *		2015-04-12  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
RC checkRID(RM_TableData *rel, RID id) {
	printf("BEGIN-checkRID\n");
	int totalPages = getTotalPages(rel);

	if (totalPages < 0) {
		return totalPages;
	}

	int pageMaxRecords = getPageMaxRecords(rel);

	if (pageMaxRecords < 0) {
		return pageMaxRecords;
	}

	if (id.page < 0 || id.page > totalPages - 1 || id.slot < 0
			|| id.slot > pageMaxRecords - 1) {
		return RC_RID_OUT_OF_BOUND;
	}

	int dataPages = RESERVED_PAGE_CAPACITY + FIRST_RESERVED_PAGE;

	if ((id.page % dataPages) == FIRST_RESERVED_PAGE) {
		return RC_RID_IS_RESERVED_PAGE;
	}

	printf("END\n\n");
	return RC_OK;
} // checkRID

/**************************************************************************************
 * Function Name: getReservedPageNum
 *
 * Description:
 *		get the reserved page number that records the passed in RID
 *
 * Parameters:
 * 		RID id: this RID we want to check
 *
 * Return:
 *		PageNumber: the reserved page's number
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-12  Jie Zhou <jzhou49@hawk.iit.edu>	Initialization
 **************************************************************************************/
PageNumber getReservedPageNum(PageNumber pageNum) {
	printf("BEGIN-getReservedPageNum\n");
	PageNumber reservedPageNum;

	int dataPages = RESERVED_PAGE_CAPACITY + FIRST_RESERVED_PAGE;

	div_t d = div(pageNum, dataPages);

	if (d.rem == 0) {
		reservedPageNum = (d.quot - 1) * dataPages + FIRST_RESERVED_PAGE;
	} else {
		reservedPageNum = d.quot * dataPages + FIRST_RESERVED_PAGE;
	}

	printf("END\n\n");
	return reservedPageNum;
} // getReservedPageNum

/**************************************************************************************
 * Function Name: isPageFree
 *
 * Description:
 *		check the passed in page has empty slots or not
 *
 * Parameters:
 * 		RM_TableData *rel: input table data
 * 		PageNumber reservedPageNum: reserved page number for the passed in page
 * 		PageNumber pageNum: the page we want to check
 *
 * Return:
 *		bool: has empty slots or not
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name									Content
 *		----------  --------------------------------------	------------------------
 *		2015-04-12  Chengnan Zhao <czhao18@hawk.iit.edu>	Initialization
 **************************************************************************************/
bool isPageFree(RM_TableData *rel, PageNumber reservedPageNum,
		PageNumber pageNum) {
	printf("BEGIN-isPageFree\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	int bitOffset = RESERVED_PAGE_HEADER * sizeof(char) * 8 + pageNum
			- reservedPageNum;

	// pin page reservedPageNum which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	bool isFree = getBit(page->data, bitOffset);

	// release the authority of handling page reservedPageNum
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return isFree;
} // isPageFree

/**************************************************************************************
 * Function Name: getBit
 *
 * Description:
 *		Get a bit value (TRUE/FALSE)
 *
 * Parameters:
 *		char *data: data entry
 *		int bitOffset: bit offset
 *
 * Return:
 *		bool: TRUE/FALSE
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
bool getBit(char *data, int bitOffset) {
	printf("BEGIN-getBit\n");
	int byteOffset;
	int bitPosition;
	bool bitFlag;

	// Calculate bitOffset / (sizeof(char) * 8) and get quot and rem
	div_t divResult = div(bitOffset, sizeof(char) * 8);

	// Set offsets
	if (divResult.rem == 0) {
		byteOffset = divResult.quot - 1;
		bitPosition = divResult.rem;
	} else {
		byteOffset = divResult.quot;
		bitPosition = sizeof(char) * 8 - divResult.rem;
	}

	char byteValue = data[byteOffset];
	char bitValue = (byteValue >> bitPosition) & 1;

	if (bitValue == 0x00) {
		bitFlag = FALSE;
	} else if (bitValue == 0x01) {
		bitFlag = TRUE;
	} else {
		return -1;
	}

	printf("END\n\n");
	return bitFlag;
} // getBit

/**************************************************************************************
 * Function Name: setBit
 *
 * Description:
 *		Set a bit value (TRUE/FALSE)
 *
 * Parameters:
 *		char *data: data entry
 *		int bitOffset: bit offset
 *		bool value: the value to be set (TRUE/FALSE)
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC setBit(char *data, int bitOffset, bool value) {
	printf("BEGIN-setBit\n");
	int byteOffset;
	int bitPosition;
	char target;

	// Calculate bitOffset / (sizeof(char) * 8) and get quot and rem
	div_t divResult = div(bitOffset, sizeof(char) * 8);

	// Set byte offsets (start from 0) and bit position in the byte (start from 1 to 8)
	if (divResult.rem == 0) {
		byteOffset = divResult.quot - 1;
		bitPosition = divResult.rem;
	} else {
		byteOffset = divResult.quot;
		bitPosition = sizeof(char) * 8 - divResult.rem;
	}

	// Get the byte value to set the bit value in the bit position of that byte
	char byteValue = data[byteOffset];

	// Set target value
	if (value == TRUE) {
		target = 0x01;
	} else if (value == FALSE) {
		target = 0x00;
	} else {
		return RC_INVALID_TARGET_VALUE;
	}

	// TODO - Add details of the logic here later
	char resultValue = (byteValue & ~(1 << (bitPosition)))
			| (target << (bitPosition));

	// Set bit value
	data[byteOffset] = resultValue;

	printf("END\n\n");
	return RC_OK;
} // setBit

/**************************************************************************************
 * Function Name: createReservedPage
 *
 * Description:
 *		pin the reserved page and initialize the page
 *
 * Parameters:
 *		PageNumber pageNum: page number pinned
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
 *		2015-04-08  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
RC createReservedPage(RM_TableData *rel, PageNumber reservedPageNum) {
	printf("BEGIN-createReservedPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	memset(page->data, 0xFF, PAGE_SIZE);

	int *ip = (int *) page->data;

	ip[0] = reservedPageNum + 1;
	ip[1] = 0;

	if (reservedPageNum == FIRST_RESERVED_PAGE) {
		ip[2] = 0;
	} else {
		ip[2] = reservedPageNum
				- (RESERVED_PAGE_CAPACITY + FIRST_RESERVED_PAGE);

		rc = -99;
		rc = setNextReservedPage(rel, ip[2], reservedPageNum);

		if (rc != RC_OK) {
			return rc;
		}
	}

	char *cp = page->data + sizeof(int) * 3;
	memset(cp, '#', RESERVED_PAGE_HEADER - sizeof(int) * 3);

	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// increment totalPages by one
	int totalPages = getTotalPages(rel);

	if (totalPages < 0) {
		return totalPages;
	}

	rc = -99;
	rc = setTotalPages(rel, ++totalPages);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // createReservedPage

/**************************************************************************************
 * Function Name: updateReservedPage
 *
 * Description:
 *		update the reserved page and initial
 *
 * Parameters:
 *		PageNumber pageNum: reserved page number
 *		PageNumber PageNum: page number with/without free page
 *		boolean isFree: 1/0
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
 *		2015-04-09  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
RC updateReservedPage(RM_TableData *rel, PageNumber reservedPageNum,
		PageNumber pageNum, bool value) {
	printf("BEGIN-updateReservedPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// find the bit corresponding to the page
	int bitOffset = RESERVED_PAGE_HEADER * sizeof(char) * 8 + pageNum
			- reservedPageNum;

	// pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// set bit to isFree
	rc = -99;
	rc = setBit(page->data, bitOffset, value);

	PageNumber firstFreePageNum = getFirstFreePage(rel, reservedPageNum);

	if (firstFreePageNum < 0) {
		return firstFreePageNum;
	}

	// if the value to be set is FALSE (page becomes full) and the pageNum <= firstFreePage,
	// then find the next first free page's number (start from the pageNum's bit) and reset the firstFreePage
	if (value == FALSE && pageNum <= firstFreePageNum) {
		// Find the next first free page
		// Set the pointer to the first bit after the pageNumber's bit
		if (firstFreePageNum != (reservedPageNum + RESERVED_PAGE_CAPACITY)) {
			bitOffset++;
		}

		// Set the firstFreePage
		int maxBitOffset = PAGE_SIZE * sizeof(char) * 8;

		while (bitOffset < maxBitOffset && getBit(page->data, bitOffset) == 0) {
			bitOffset++;
		}

		if (bitOffset == maxBitOffset && getBit(page->data, bitOffset) == 0) {
			int totalPages = getTotalPages(rel);

			if (totalPages < 0) {
				return totalPages;
			}

			rc = -99;
			rc = createReservedPage(rel, totalPages);

			if (rc != RC_OK) {
				return rc;
			}

			rc = -99;
			rc = setTotalPages(rel, ++totalPages);

			if (rc != RC_OK) {
				return rc;
			}

			rc = -99;
			rc = setFirstFreePage(rel, reservedPageNum, 0);

			if (rc != RC_OK) {
				return rc;
			}
		} else {
			PageNumber nextFreePageNum = bitOffset
					- RESERVED_PAGE_HEADER * sizeof(char) * 8 + reservedPageNum;
			rc = -99;
			rc = setFirstFreePage(rel, reservedPageNum, nextFreePageNum);

			if (rc != RC_OK) {
				return rc;
			}
		}
	}

	// unpin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // updateReservedPage

/**************************************************************************************
 * Function Name: getFirstFreePage
 *
 * Description:
 *		Get the first free page number in reserved page
 *
 * Parameters:
 *		char *data:
 *
 * Return:
 *		int: page number
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-013  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
PageNumber getFirstFreePage(RM_TableData *rel, PageNumber reservedPageNum) {
	printf("BEGIN-getFirstFreePage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data;
	int firstFreePage = ip[0];

	// unpin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return firstFreePage;
} // getFirstFreePage

/**************************************************************************************
 * Function Name: setFirstPageNum
 *
 * Description:
 *		set the first num in reserved page
 *
 * Parameters:
 *		char *data:
 *		int num:
 *
 * Return:
 *		RC: RC_OK
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
RC setFirstFreePage(RM_TableData *rel, PageNumber reservedPageNum, int value) {
	printf("BEGIN-setFirstFreePage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;
	ip[0] = value;

	// unpin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setFirstFreePage

/**************************************************************************************
 * Function Name: getNextReservedPage
 *
 * Description:
 *		get the next reserved page number
 *
 * Parameters:
 *		char *data:
 *
 * Return:
 *		int: page number
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
PageNumber getNextReservedPage(RM_TableData *rel, PageNumber reservedPageNum) {
	printf("BEGIN-getNextReservedPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data;
	int nextReservedPage = ip[1];

	// unpin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return nextReservedPage;
} // getNextReservedPage

/**************************************************************************************
 * Function Name: setNextReservedPage
 *
 * Description:
 *		set the second num in reserved page
 *
 * Parameters:
 *		char *data:
 *		int num:
 *
 * Return:
 *		RC: RC_OK
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
RC setNextReservedPage(RM_TableData *rel, PageNumber reservedPageNum, int value) {
	printf("BEGIN-setNextReservedPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;
	ip[1] = value;

	// unpin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setNextReservedPage

/**************************************************************************************
 * Function Name: getPrevReversedPage
 *
 * Description:
 *		get the previous reserved page
 *
 * Parameters:
 *		char *data:
 *
 * Return:
 *		int: page number
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-013  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
PageNumber getPrevReservedPage(RM_TableData *rel, PageNumber reservedPageNum) {
	printf("BEGIN-getPrevReservedPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data;
	int prevReservedPage = ip[2];

	// pin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return prevReservedPage;
} // getPrevReservedPage

/**************************************************************************************
 * Function Name: setPrevReservedPage
 *
 * Description:
 *		set the second num in reserved page
 *
 * Parameters:
 *		char *data:
 *		int num:
 *
 * Return:
 *		RC: RC_OK
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
RC setPrevReservedPage(RM_TableData *rel, PageNumber reservedPageNum, int value) {
	printf("BEGIN-setPrevReservedPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// Pin the reserved page
	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;
	ip[2] = value;

	// Unpin the reserved page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setPrevReservedPage

/**************************************************************************************
 * Function Name: searchFirstFreePage
 *
 * Description:
 *		search for the first free page number
 *
 * Parameters:
 *		RM_TableData *rel:
 *
 * Return:
 *		int: page number
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
PageNumber searchFirstFreePage(RM_TableData *rel) {
	printf("BEGIN-searchFirstFreePage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	PageNumber firstFreePage;
	PageNumber reservedPageNum = FIRST_RESERVED_PAGE;

	rc = -99;
	rc = pinPage(BM, page, reservedPageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) page->data;

	while (ip[0] == 0 && ip[1] != 0) {
		rc = -99;
		rc = unpinPage(BM, page);

		if (rc != RC_OK) {
			return rc * (-1);
		}

		reservedPageNum += RESERVED_PAGE_CAPACITY + FIRST_RESERVED_PAGE;

		rc = -99;
		rc = pinPage(BM, page, reservedPageNum);

		if (rc != RC_OK) {
			return rc * (-1);
		}

		ip = (int *) page->data;
	}

	if (ip[0] == 0 && ip[1] == 0) {
		// Set firstFreePage to -1 as error code
		firstFreePage = -1;
	} else {
		firstFreePage = ip[0];
	}

	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return firstFreePage;
} // searchFirstFreePage

/**************************************************************************************
 * Function Name: isPageInitialized
 *
 * Description:
 *		Get isPageInitialized (TRUE/FALSE)
 *
 * Parameters:
 *		char *data: data entry
 *
 * Return:
 *		bool: isPageInitialized (TRUE/FALSE)
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
bool isPageInitialized(RM_TableData *rel, PageNumber pageNum) {
	printf("BEGIN-isPageInitialized\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	char value = *page->data;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	bool isInited;

	if (value == '1') {
		isInited = TRUE;
	} else if (value == 0) {
		isInited = FALSE;
	} else {
		return -1;
	}

	free(page);

	printf("END\n\n");
	return isInited;
} // isPageInitialized

/**************************************************************************************
 * Function Name: setPageInitialized
 *
 * Description:
 *		Set isPageInitializedd (TRUE/FALSE)
 *
 * Parameters:
 *		char *data: data entry
 *		bool value: value to be set (TRUE/FALSE)
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC setPageInitialized(RM_TableData *rel, PageNumber pageNum, bool value) {
	printf("BEGIN-setPageInitialized\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// Get target value
	char target;

	if (value == TRUE) {
		target = '1';
	} else if (value == FALSE) {
		target = 0;
	} else {
		return RC_INVALID_TARGET_VALUE;
	}

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	*page->data = target;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setPageInitialized

/**************************************************************************************
 * Function Name: getCurrentRecords
 *
 * Description:
 *		Get currentRecords
 *
 * Parameters:
 *		char *data: data entry
 *
 * Return:
 *		int: currentRecords
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
int getCurrentRecords(RM_TableData *rel, PageNumber pageNum) {
	printf("BEGIN-getCurrentRecords\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	int *ip = (int *) (page->data + 1);
	int currentRecords = ip[0];

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return currentRecords;
} // getCurrentRecords

/**************************************************************************************
 * Function Name: setCurrentRecords
 *
 * Description:
 *		Set currentRecords
 *
 * Parameters:
 *		char *data: data entry
 *		int value: value to be set
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC setCurrentRecords(RM_TableData *rel, PageNumber pageNum, int value) {
	printf("BEGIN-setCurrentRecords\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) (page->data + 1);
	ip[0] = value;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setCurrentRecords

/**************************************************************************************
 * Function Name: isSlotOccupied
 *
 * Description:
 *		Get isSlotOccupied (TRUE/FALSE)
 *
 * Parameters:
 *		char *data: data entry
 *
 * Return:
 *		bool: isSlotOccupied (TRUE/FALSE)
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
bool isSlotOccupied(RM_TableData *rel, PageNumber pageNum, int slotId) {
	printf("BEGIN-isSlotOccupied\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	char *cp = page->data + sizeof(char) + sizeof(int) + sizeof(char) * slotId;
	char value = *cp;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	bool isOccupied;

	if (value == '1') {
		isOccupied = TRUE;
	} else if (value == 0) {
		isOccupied = FALSE;
	} else {
		return -1;
	}

	free(page);

	printf("END\n\n");
	return isOccupied;
} // isSlotOccupied

/**************************************************************************************
 * Function Name: setSlotOccupied
 *
 * Description:
 *		Set isSlotOccupied (TRUE/FALSE)
 *
 * Parameters:
 *		char *data: data entry
 *		int slotId: slot id
 *		bool value: value to be set (TRUE/FALSE)
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC setSlotOccupied(RM_TableData *rel, PageNumber pageNum, int slotId,
		bool value) {
	printf("BEGIN-setSlotOccupied\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// Get target value
	char target;

	if (value == TRUE) {
		target = '1';
	} else if (value == FALSE) {
		target = 0;
	} else {
		return RC_INVALID_TARGET_VALUE;
	}

	// pin page 0 which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	char *cp = page->data + sizeof(char) + sizeof(int) + sizeof(char) * slotId;
	*cp = target;

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // setSlotOccupied

/**************************************************************************************
 * Function Name: checkRecordLength
 *
 * Description:
 *		Check if the length of the record is valid
 *
 * Parameters:
 *		char *recordData: record data entry
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC checkRecordLength(RM_TableData *rel, char *recordData) {
	printf("BEGIN-checkRecordLength\n");
	int recordLength = getRecordLength(rel);

	if (recordLength < 0) {
		return recordLength;
	}

	if (recordLength != strlen(recordData)) {
		return RC_CHECK_RECORD_LENGTH_ERROR;
	}

	printf("END\n\n");
	return RC_OK;
} // checkRecordLength

/**************************************************************************************
 * Function Name: initRecordPage
 *
 * Description:
 *		initialize the new record page
 *
 * Parameters:
 *		char *data: data entry
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
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC initRecordPage(RM_TableData *rel, PageNumber pageNum) {
	printf("BEGIN-initRecordPage\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// Set isPageInitialized to TRUE
	rc = -99;
	rc = setPageInitialized(rel, pageNum, TRUE);

	if (rc != RC_OK) {
		return rc;
	}

	// Set currentRecords to 0
	rc = -99;
	rc = setCurrentRecords(rel, pageNum, 0);

	if (rc != RC_OK) {
		return rc;
	}

	int pageMaxRecords = getPageMaxRecords(rel);

	// pin page with pageNum which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	char *cp = page->data + sizeof(char) + sizeof(int);

	memset(cp, 0, pageMaxRecords);

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // initRecordPage

/**************************************************************************************
 * Function Name: getFreeSlotId
 *
 * Description:
 *		Get the first free slot's id
 *
 * Parameters:
 *		char *data: data entry
 *
 * Return:
 *		int: slotId
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
int getFreeSlotId(RM_TableData *rel, PageNumber pageNum) {
	printf("BEGIN-getFreeSlotId\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	int pageMaxRecords = getPageMaxRecords(rel);

	if (pageMaxRecords < 0) {
		return pageMaxRecords;
	}

	// pin page with pageNum which stores the information of input table
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	char *cp = page->data + sizeof(char) + sizeof(int);

	int i = 0;
	while (i < pageMaxRecords && *(cp + i * sizeof(char)) != 0) {
		i++;
	}

	int slotId;

	if (i < pageMaxRecords && *(cp + i * sizeof(char)) == 0) {
		slotId = i;
	} else if (i == pageMaxRecords) {
		return RC_PAGE_FULL_ERROR * (-1);
	}

	// release the authority of handling page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc * (-1);
	}

	free(page);

	printf("END\n\n");
	return slotId;
} // getFreeSlotId

// handling records in a table
/**************************************************************************************
 * Function Name: insertRecord
 *
 * Description:
 *		Insert record
 *
 * Parameters:
 *		char *data: data entry
 *
 * Return:
 *		int: slotId
 *
 * Author:
 *		Xin Su <xsu11@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                Content
 *		----------  ----------------------------------  ------------------------
 *		2015-04-08  Xin Su <xsu11@hawk.iit.edu>         Initialization
 **************************************************************************************/
RC insertRecord(RM_TableData *rel, Record *record) {
	printf("BEGIN-insertRecord\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc; // return code

	int slotId;
	int spaceOffset;

	int recordLength = getRecordLength(rel);

	if (recordLength < 0) {
		return recordLength;
	}

	int pageMaxRecords = getPageMaxRecords(rel);

	if (pageMaxRecords < 0) {
		return pageMaxRecords;
	}

	int totalRecords = getTotalRecords(rel);

	if (totalRecords < 0) {
		return totalRecords;
	}

	// Search the first page with free space
	PageNumber pageNum = searchFirstFreePage(rel);

	// The record page with this pageNum must not be full,
	// that is, if this record page is initialized, then currentRecords < pageMaxRecords
	// else if this page is not, then initialize it and set currentRecords to 0
	if (pageNum < 0) {
		return pageNum;
	}

	// Check if this record page is initialized
	bool isInited = isPageInitialized(rel, pageNum);

	// if is is not initialized, then initialize it and set slotId to 0
	// else if it is, then simply get slotId
	// else, some error happens, return rc
	if (isInited < 0) {
		return isInited;
	} else if (isInited == FALSE) {
		rc = -99;
		rc = initRecordPage(rel, pageNum);

		if (rc != RC_OK) {
			return rc;
		}

		slotId = 0;

		// increment totalPages by one
		int totalPages = getTotalPages(rel);

		if (totalPages < 0) {
			return totalPages;
		}

		rc = -99;
		rc = setTotalPages(rel, ++totalPages);

		if (rc != RC_OK) {
			return rc;
		}

	} else if (isInited == TRUE) {
		slotId = getFreeSlotId(rel, pageNum);

		if (slotId < 0) {
			return slotId;
		}
	}

	// Get spaceOffset by slotId
	spaceOffset = PAGE_SIZE - (slotId + 1) * recordLength * sizeof(char);

	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	// Get the free space position and insert record
	char *cp = page->data + spaceOffset;

	memcpy(cp, record->data, recordLength);

	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// Get currentRecords
	int currentRecords = getCurrentRecords(rel, pageNum);

	if (currentRecords < 0) {
		return currentRecords;
	}

	// Increment currentRecords by one and then set it
	rc = -99;
	rc = setCurrentRecords(rel, pageNum, ++currentRecords);

	if (rc != RC_OK) {
		return rc;
	}

	// Set the isOccupied flag of the slot to TRUE
	rc = -99;
	rc = setSlotOccupied(rel, pageNum, slotId, TRUE);

	if (rc != RC_OK) {
		return rc;
	}

	// increment totalRecords in the table info page by one
	rc = -99;
	rc = setTotalRecords(rel, ++totalRecords);

	if (rc != RC_OK) {
		return rc;
	}

	// No need to check the return value
	PageNumber reservedPageNum = getReservedPageNum(pageNum);

	// if now the currentRecords = pageMaxRecords, then the page is full, set the isFree flag in the reserved page to 0
	if (currentRecords == pageMaxRecords) {
		rc = -99;
		rc = updateReservedPage(rel, reservedPageNum, pageNum, FALSE);

		if (rc != RC_OK) {
			return rc;
		}
	}

	// Set the record's RID
	record->id.page = pageNum;
	record->id.slot = slotId;

	free(page);

	printf("END\n\n");
	return RC_OK;
} // insertRecord

/**************************************************************************************
 * Function Name: deleteRecord
 *
 * Description:
 *		Delete record
 *
 * Parameters:
 * 		RM_TableData *rel : table: table enty
 *		RID id: RID of the record we about to delete
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Chengnan Zhao <czhao18@hawk.iit.edu>
 *
 * History:
 *		Date        Name                               		Content
 *		----------  ----------------------------------  	------------------------
 *		2015-04-08  Chengnan Zhao <czhao18@hawk.iit.edu>    Initialization
 **************************************************************************************/
RC deleteRecord(RM_TableData *rel, RID id) {
	printf("BEGIN-deleteRecord\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	rc = -99;
	rc = checkRID(rel, id);

	if (rc != RC_OK) {
		return rc;
	}

	// The RID is valid. Now remove the content stored in RID

	// Get recordLength
	int recordLength = getRecordLength(rel);

	if (recordLength < 0) {
		return recordLength;
	}

	rc = -99;
	rc = pinPage(BM, page, id.page);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// Set recordOffset
	int spaceOffset = PAGE_SIZE - sizeof(char) * recordLength * (id.slot + 1);

	// Delete the record by set all bytes to 0
	char *cp = page->data + spaceOffset;
	memset(cp, 0, recordLength);

	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// Decrement currentRecords stored in id.page
	int currentRecords = getCurrentRecords(rel, id.page);

	if (currentRecords < 0) {
		return currentRecords;
	}

	rc = -99;
	rc = setCurrentRecords(rel, id.page, --currentRecords);

	if (rc != RC_OK) {
		return rc;
	}

	// Now the currentRecords is the new value

	// Set isOccupied to FALSE
	rc = -99;
	rc = setSlotOccupied(rel, id.page, id.slot, FALSE);

	if (rc != RC_OK) {
		return rc;
	}

	// Decrement the totalRecords stored in page 0
	int totalRecords = getTotalRecords(rel);

	if (totalRecords < 0) {
		return totalRecords;
	}

	rc = -99;
	rc = setTotalRecords(rel, --totalRecords);

	if (rc != RC_OK) {
		return rc;
	}

	// Now the totalRecords is the new value

	// No need to check the return value for this function
	PageNumber reservedPageNum = getReservedPageNum(id.page);

	// Get the pageMaxRecords
	int pageMaxRecords = getPageMaxRecords(rel);

	if (pageMaxRecords < 0) {
		return pageMaxRecords;
	}

	// if now the currentRecords = pageMaxRecords - 1, then the page becomes free
	// update the isFree flag in the reserved page to TRUE
	if (currentRecords == pageMaxRecords - 1) {
		rc = -99;
		rc = updateReservedPage(rel, reservedPageNum, id.page, TRUE);

		if (rc != RC_OK) {
			return rc;
		}
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // deleteRecord

/**************************************************************************************
 * Function Name: updateRecord
 *
 * Description:
 *		write info into page
 *
 * Parameters:
 *		RM_TableData *rel:
 *		Record *record:
 *
 * Return:
 *		int: page number
 *
 * Author:
 *		Jie Zhou <jzhou49@hawk.iit.edu>
 *
 * History:
 *		Date        Name								Content
 *		----------  ----------------------------------	----------------------------
 *		2015-04-13  Jie Zhou <jzhou49@hawk.iit.edu>		Initialization
 **************************************************************************************/
RC updateRecord(RM_TableData *rel, Record *record) {
	printf("BEGIN-updateRecord\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	bool isOccupied = isSlotOccupied(rel, record->id.page, record->id.slot);

	if (isOccupied < 0) {
		return isOccupied;
	} else if (isOccupied == FALSE) {
		return RC_SLOT_EMPTY;
	} else if (isOccupied != TRUE) {
		return RC_SLOT_ERROR;
	}

	rc = -99;
	rc = pinPage(BM, page, record->id.page);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = markDirty(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	int recordLength = getRecordLength(rel);

	if (recordLength < 0) {
		return recordLength;
	}

	// Set the offset
	int spaceOffset = PAGE_SIZE
			- (record->id.slot + 1) * recordLength * sizeof(char);

	char *cp = page->data + spaceOffset;

	// write the data into page
	memcpy(cp, record->data, recordLength);

	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	printf("END\n\n");
	return RC_OK;
} // updateRecord

/**************************************************************************************
 * Function Name: getRecord
 *
 * Description:
 *		Get a record from certain table with certain RID
 *		if not found, return RC_NON_EXISTING_RECORD
 *
 * Parameters:
 *		RM_TableData * rel: a pointer to the table
 *		RID id: id of the record we want to get
 *		Record * record: a pointer to the record
 *
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-10  Xiaolang Wang <xwang122@hawk.iit.edu>     1. change the last return to RC_OK
 *															  2. add unpinPage after pin
 *		2015-04-12  Xiaolang Wang <xwang122@hawk.iit.edu>	  TOTAL_RECORDS is changed to
 *															  PAGE_MAX_RECORDS
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>	  add check the TOTAL_PAGES
 **************************************************************************************/
RC getRecord(RM_TableData *rel, RID id, Record *record) {
	printf("BEGIN-getRecord\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	rc = -99;
	rc = checkRID(rel, id);

	if (rc != RC_OK) {
		return rc;
	}

	// check if the slot is occupied or not
	bool isOccupied = isSlotOccupied(rel, id.page, id.slot);

	// if not occupied, return RC_NON_EXISTING_RECORD

	if (isOccupied < 0) {
		return isOccupied;
	} else if (isOccupied == FALSE) {
		return RC_SLOT_EMPTY;
	} else if (isOccupied != TRUE) {
		return RC_SLOT_ERROR;
	}

	// declare integers to store RECORD_LENGTH, PAGE_MAX_RECORDS, TOTAL_PAGES
	int recordLength = getRecordLength(rel);

	if (recordLength < 0) {
		return recordLength;
	}

	// start place of the record we need
	int spaceOffset = PAGE_SIZE - (id.slot + 1) * recordLength * sizeof(char);

	rc = -99;
	rc = pinPage(BM, page, id.page);

	if (rc != RC_OK) {
		return rc;
	}

	char *cp = page->data + spaceOffset;

	// get this record at the slot
	// notice, we record our records from the bottom of a page to the top
	memcpy(record->data, cp, recordLength);

	// add a \0 at the end of cp
	record->data[recordLength] = '\0';

	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	//if there is a record, let record's id = id passed in
	record->id.page = id.page;
	record->id.slot = id.slot;

	free(page);

	printf("END\n\n");
	return RC_OK;
} // getRecord

// scans
/**************************************************************************************
 * Function Name: startScan
 *
 * Description:
 *		initialize a ScanManager with a given table and given restrict
 *
 * Parameters:
 *		RM_TableData * rel: a pointer to the table
 *		RM_ScanHandle *scan: a ScabHandle that helps scanning
 *		Expr *cond: the expression of the restrict
 *
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-10  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>     delete the temporary ScanHandler
 **************************************************************************************/
RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
	printf("BEGIN-startScan\n");

	// allocate memory for RM_ScanCondition
	RM_ScanCondition *sc = (RM_ScanCondition *) malloc(
			sizeof(RM_ScanCondition));
	sc->id = (RID *) malloc(sizeof(RID));
	sc->cond = cond;
	sc->id->page = FIRST_RESERVED_PAGE + 1;
	sc->id->slot = 0;

	// let the variables point to the input values

	scan->rel = rel;
	scan->mgmtData = sc;

	/*
	 if (R_ID == NULL) {
	 R_ID = (RID *) malloc(sizeof(RID));
	 }

	 R_ID->page = FIRST_RESERVED_PAGE + 1;
	 R_ID->slot = -1;
	 */

	printf("END\n\n");
	return RC_OK;
} // startScan

/**************************************************************************************
 * Function Name: next
 *
 * Description:
 *		return the next eligible record
 *
 * Parameters:
 *		RM_ScanHandle *scan: the ScanHandle we are using now
 *		Record *record: to pointer points to the record we get
 *
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC next(RM_ScanHandle *scan, Record *record) {
	printf("BEGIN-next\n");
	RC rc;

	RM_ScanCondition *sc = (RM_ScanCondition *) scan->mgmtData;

	if (sc->id == NULL) {
		return RC_NO_RID;
	}

	int totalPages = getTotalPages(scan->rel);

	if (totalPages < 0) {
		return totalPages;
	}

	int totalRecords = getTotalRecords(scan->rel);

	if (totalRecords < 0) {
		return totalRecords;
	}

	int pageMaxRecords = getPageMaxRecords(scan->rel);

	if (pageMaxRecords < 0) {
		return pageMaxRecords;
	}

	int dataPages = RESERVED_PAGE_CAPACITY + FIRST_RESERVED_PAGE;

	// if to the end ,return
	div_t d = div(sc->id->page, dataPages);

	int curRecordPos = d.quot
			* (RESERVED_PAGE_CAPACITY + d.rem - FIRST_RESERVED_PAGE - 1)
			* pageMaxRecords + (sc->id->slot + 1);

	if (curRecordPos == totalRecords) {
		// set RECORD back to the first record position
		sc->id->page = FIRST_RESERVED_PAGE + 1;
		sc->id->slot = -1;

		return RC_RM_NO_MORE_TUPLES;
	}

	/*
	 * If the code comes here, then the scan does not comes to the end
	 * RECORD->id.page <= (totalPages - 1)
	 * We need to set the rid to the next record
	 */

	if (sc->id->slot == (pageMaxRecords - 1)) {
		if (sc->id->page == totalPages - 1) {
			// set RECORD back to the first record position
			sc->id->page = FIRST_RESERVED_PAGE + 1;
			sc->id->slot = -1;

			return RC_RM_NO_MORE_TUPLES;
		}

		curRecordPos++;

		// set to the first slot in the next page
		sc->id->page++;
		sc->id->slot = 0;
	} else {
		curRecordPos++;

		// set to the next slot in this page
		sc->id->slot++;
	}

	// declare an array of Value to store the expression evaluated result
	// the first Value in the array is a pointer to a boolean Value
	Value *isFound = (Value *) malloc(sizeof(Value));
	isFound->v.boolV = FALSE;
	isFound->dt = DT_BOOL;

	Value **result = (Value **) malloc(sizeof(Value *));
	*result = isFound;

	// a nest while loop, that starts from rid till the end of file
	// or the wanted record is found
	// the outer loop is for pages
	while (curRecordPos < (totalRecords + 1) && sc->id->page < totalPages
			&& (*result)->v.boolV == FALSE) {
		// search this page only if it's not a reserved page
		if ((sc->id->page % dataPages) != FIRST_RESERVED_PAGE
				&& sc->id->slot != (pageMaxRecords - 1)) {
			// inner loop for slots on one page
			while (curRecordPos < (totalRecords + 1)
					&& sc->id->slot < pageMaxRecords
					&& (*result)->v.boolV == FALSE) {
				// check is the current slot is occupied
				// we check if the record in this slot only if it's occupied
				bool isOccupied = isSlotOccupied(scan->rel, sc->id->page,
						sc->id->slot);

				if (isOccupied < 0) {
					return isOccupied;
				} else if (isOccupied == TRUE) {
					// get the record at rid
					rc = -99;
					rc = getRecord(scan->rel, *(sc->id), record);

					if (rc != RC_OK) {
						return rc;
					}

					// evaluate this record with the given schema, and update isFound
					// if something wrong happened, return rc, set isFound = FALSE
					rc = -99;
					rc = evalExpr(record, scan->rel->schema, sc->cond, result);

					if (rc != RC_OK) {
						return rc;
					}

					// check isFound still a boolean, if not, set it back to boolean and FALSE again
					// if ((*result)->dt != DT_BOOL) {
					if ((*result)->v.boolV == FALSE) {
						free(*result);
						*result = isFound;
					}
				} // end of if "is it occupied"

				// increase slot number
				if ((*result)->v.boolV == FALSE) {
					curRecordPos++;
					sc->id->slot++;
				}

			} // end of inner while loop
		} // end of if "is it reserved page"

		/*
		 * If the code comes here,
		 * the either: the pageNum is a reserved page
		 * or: sc->id->slot = pageMaxRecords: we need to move to the next page
		 * or: (*result)->v.boolV == TRUE: we found the record and need to jump out the outside while loop
		 * (using the outside while loop check condition)
		 */

		//increase page number and reset slot number
		if ((*result)->v.boolV == FALSE) {
			curRecordPos++;
			sc->id->page++;
			sc->id->slot = 0;
		}
	} // end of the outer while loop

	if ((*result)->v.boolV == TRUE) {
		rc = RC_OK;
	} else if (curRecordPos == totalRecords || sc->id->page == totalPages) {
		sc->id->page = record->id.page;
		sc->id->slot = record->id.slot;
		rc = RC_RM_NO_MORE_TUPLES;
	}

	free(isFound);
	free(result);

	printf("END\n\n");
	return rc;
} // next

/**************************************************************************************
 * Function Name: closeScan
 *
 * Description:
 *		Unset the scan manager
 *
 * Parameters:
 *		RM_ScanHandle *scan: the ScanHandle we are using now
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC closeScan(RM_ScanHandle *scan) {
	printf("BEGIN-closeScan\n");
	RM_ScanCondition *sc = (RM_ScanCondition *) scan->mgmtData;
	free(sc->id);
	free(sc);

	scan->mgmtData = NULL;
	scan->rel = NULL;

	// R_ID->page = FIRST_RESERVED_PAGE + 1;
	// R_ID->slot = -1;

	printf("END\n\n");
	return RC_OK;
} // closeScan

// dealing with schemas
/**************************************************************************************
 * Function Name: getRecordSize
 *
 * Description:
 *		get the size of the each record for a given schema
 *
 * Parameters:
 *		Schema *schema: the schema we want to know size
 *
 * Return:
 *		int: size of each record of this schema
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-07  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
int getRecordSize(Schema *schema) {
	printf("BEGIN-getRecordSize\n");
	int recordSize = 0;

	// loop to calculate the record size
	int i;
	for (i = 0; i < schema->numAttr; i++) {
		if (schema->dataTypes[i] == DT_INT) {
			schema->typeLength[i] = sizeof(int);
		} else if (schema->dataTypes[i] == DT_FLOAT) {
			schema->typeLength[i] = sizeof(float);
		} else if (schema->dataTypes[i] == DT_BOOL) {
			schema->typeLength[i] = sizeof(bool);
		} else if (schema->dataTypes[i] != DT_STRING) {
			return RC_WRONG_DATATYPE * (-1);
		}

		recordSize += schema->typeLength[i];
	}

	printf("END\n\n");
	return recordSize;
} // getRecordSize

/**************************************************************************************
 * Function Name: schemaToString
 *
 * Description:
 *		parse the schema into char *
 *
 * Parameters:
 *		Schema *schema: the schema we want to know size
 *
 * Return:
 *		char * : all info of schema stored in a string
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-08  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>     1. function name is changed to start
 *														      with a small letter
 *														      2. add a "\0" at the end
 **************************************************************************************/
char *schemaToString(Schema * schema) {
	printf("BEGIN-schemaToString\n");
	VarString *schemaStr; // a VarString variable schema_str to record the Schema

	// I will use the VarString structure and its functions
	// "|" between variables of this Schema, "," between attributes for each variable
	MAKE_VARSTRING(schemaStr); 	// make it;
	APPEND(schemaStr, "%d%c", schema->numAttr, '|'); // add numAttr and a '|' between variables

	int i;
	// use a for loop to add attrName
	for (i = 0; i < schema->numAttr; i++) {
		// ',' between attributes and '|' at last
		APPEND(schemaStr, "%s%c", schema->attrNames[i],
				(i != schema->numAttr - 1) ? ',' : '|');
	}

	// similar for dataTypes and typeLength
	for (i = 0; i < schema->numAttr; i++) {
		// ',' between attributes and '|' at last
		APPEND(schemaStr, "%d%c", schema->dataTypes[i],
				(i != schema->numAttr - 1) ? ',' : '|');
	}

	for (i = 0; i < schema->numAttr; i++) {
		// ',' between attributes and '|' at last
		APPEND(schemaStr, "%d%c", schema->typeLength[i],
				(i != schema->numAttr - 1) ? ',' : '|');
	}

	// we use a for loop to add key attributes
	for (i = 0; i < schema->keySize; i++) {
		APPEND(schemaStr, "%d%c", schema->keyAttrs[i],
				(i != schema->keySize - 1) ? ',' : '|');
	}

	// add keySize and a '|', and \0 that indicates end
	APPEND(schemaStr, "%d%c%c", schema->keySize, '|', '\0');

	printf("END\n\n");
	RETURN_STRING(schemaStr); // return
} // schemaToString

/**************************************************************************************
 * Function Name: createSchema
 *
 * Description:
 *		create a new schema with given values and write the global variables
 *
 * Parameters:
 *		int numAttr: number of attributes
 *		char **attrNames: a pointer to an array of pointers to attribute names
 *		DataType *dataTypes: an array of data types for each attribute
 *		int *typeLength: an array of length for each attribute
 *		int keySize: number of keys
 *		int *keys: an array of the positions of the key attributes
 *
 * Return:
 *		int: size of each record of this schema
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-08  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-10  Xiaolang Wang <xwang122@hawk.iit.edu>     1. delete the PAGE_MAX_RECORDS
 *																 from the head
 *															  2. Page 0 is changed to
 *															  	 RECORD_LENGTH, PAGE_MAX_RECORDS,
 *															  	 totalRecords, SCHEMA_STR
 *		2015-04-12  Xiaolang Wang <xwang122@hawk.iit.edu>	  Page 0 is changed to
 *															  RECORD_LENGTH, TOTAL_RECORDS,
 *															  TOTAL_PAGES, PAGE_MAX_RECORDS,
 *															  and SCHEMA_STR
 **************************************************************************************/
Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes,
		int *typeLength, int keySize, int *keys) {
	printf("BEGIN-createSchema\n");

	// if the global variables are not allocated to memory, allocate first
	if (SCHEMA == NULL) {
		SCHEMA = (Schema *) malloc(sizeof(Schema));
	}

	// write values into the global variable SCHEMA
	SCHEMA->numAttr = numAttr;
	SCHEMA->attrNames = attrNames;
	SCHEMA->dataTypes = dataTypes;
	SCHEMA->typeLength = typeLength;
	SCHEMA->keyAttrs = keys;
	SCHEMA->keySize = keySize;

	// function schemaToString() will allocate memory for the return string
	// We should free
	char *result = NULL;
	result = schemaToString(SCHEMA);

	// calculate the length of schema_to_write
	int schemaLength = strlen(result);

	// check if the length is small enough. if too long, it can't be written into
	// one page, unset SCHEMA, then return NULL.
	// In table info page, there are RECORD_LENGTH, TOTAL_RECORDS,
	// TOTAL_PAGES, PAGE_MAX_RECORDS and SCHEMA_STR.
	if (schemaLength > (PAGE_SIZE - sizeof(int) * 4)) {
		memset(SCHEMA_STR, 0, PAGE_SIZE);

		SCHEMA->numAttr = -1;
		SCHEMA->attrNames = NULL;
		SCHEMA->dataTypes = NULL;
		SCHEMA->typeLength = NULL;
		SCHEMA->keyAttrs = NULL;
		SCHEMA->keySize = -1;

		free(result);

		return NULL;
	} else if (SCHEMA_STR == NULL) {
		SCHEMA_STR = (char *) calloc(PAGE_SIZE, sizeof(char));
	}

	memcpy(SCHEMA_STR, result, schemaLength);

	free(result);

	//calculate the length of a record of SCHEMA
	RECORD_LENGTH = getRecordSize(SCHEMA);

	// calculate the PAGE_MAX_RECORDS
	// On each record page, there are the following in the head
	// 1. isInitialized (1 char)
	// 2. current record number (1 int)
	// 3. isOccupied (1 char for each record)
	PAGE_MAX_RECORDS = (PAGE_SIZE - sizeof(char) - sizeof(int))
			/ (sizeof(char) + RECORD_LENGTH);

	printf("END\n\n");
	return SCHEMA;
} // createSchema

/**************************************************************************************
 * Function Name: freeSchema
 *
 * Description:
 *		unset the passed in schema
 *
 * Parameters:
 *		Schema *schema: the schema we want to free
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-08  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC freeSchema(Schema *schema) {
	printf("BEGIN-freeSchema\n");

	schema->numAttr = -1;
	schema->attrNames = NULL;
	schema->dataTypes = NULL;
	schema->typeLength = NULL;
	schema->keyAttrs = NULL;
	schema->keySize = -1;

	printf("END\n\n");
	return RC_OK;
} // freeSchema

// dealing with records and attribute values
/**************************************************************************************
 * Function Name: createRecord
 *
 * Description:
 *		create an empty record of the given schema
 *
 * Parameters:
 * 		Record **record: an array of records that we create
 *		Schema *schema: a given schema
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>     Corrected some pointer problems
 **************************************************************************************/
RC createRecord(Record **record, Schema *schema) {
	printf("BEGIN-createRecord\n");

	Record *r = (Record *) malloc(sizeof(Record));
	char *rData = (char *) calloc(RECORD_LENGTH + 1, sizeof(char));

	r->id.page = -1;
	r->id.slot = -1;
	r->data = rData;
	record[0] = r;

	printf("END\n\n");
	return RC_OK;
} // closeTable

/**************************************************************************************
 * Function Name: freeRecord
 *
 * Description:
 *		free a certain record
 *
 * Parameters:
 * 		Record *record: the record we want to free
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC freeRecord(Record *record) {
	printf("BEGIN-freeRecord\n");
	// what should this function do?

	free(record->data);
	record->data = NULL;

	free(record);
	record = NULL;

	printf("END\n\n");
	return RC_OK;
} // freeRecord

/**************************************************************************************
 * Function Name: getAttr
 *
 * Description:
 *		get the value of a given record of a given schema's attrNumth attribute and store it
 *		at the attrNumth spot in **value
 *
 * Parameters:
 * 		Record *record: a record to get attribute from
 * 		Schema *schema: the schema of the given record
 * 		int attrNum: the ordinal of the attribute we want to get from the record
 * 		Value **value: and array of pointers to store the value at certain attribute
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-10  Xiaolang Wang <xwang122@hawk.iit.edu>     correct fatal logical error
 *		2015-04-13  Xiaolang Wang <xwang122@hawk.iit.edu>     Added a global variable Value *ATTR_VALUE
 **************************************************************************************/
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
	printf("BEGIN-getAttr\n");
	// calculate the offset
	int attrOffset = 0;

	// Allocate memory for attrValue
	// Free it outside this function

	Value *attrValue = (Value *) malloc(sizeof(Value));

	// save the attrNumth attribute's DataType to ATTR_VALUE
	attrValue->dt = schema->dataTypes[attrNum];

	// loop to add the length of each attribute before the target into offset
	int i;
	for (i = 0; i < attrNum; i++) {
		attrOffset += schema->typeLength[i];
	}

	// save the attrNumth attribute to ATTR_VALUE using memcpy
	if (schema->dataTypes[attrNum] == DT_STRING) {
		int tl = schema->typeLength[attrNum];

		attrValue->v.stringV = (char *) calloc(tl + 1, sizeof(char));

		memcpy(attrValue->v.stringV, record->data + attrOffset,
				schema->typeLength[attrNum]);
		attrValue->v.stringV[tl + 1] = '\0';
	} else if (schema->dataTypes[attrNum] == DT_INT) {
		memcpy(&(attrValue->v.intV), record->data + attrOffset,
				schema->typeLength[attrNum]);
	} else if (schema->dataTypes[attrNum] == DT_FLOAT) {
		memcpy(&(attrValue->v.floatV), record->data + attrOffset,
				schema->typeLength[attrNum]);
	} else if (schema->dataTypes[attrNum] == DT_BOOL) {
		memcpy(&(attrValue->v.boolV), record->data + attrOffset,
				schema->typeLength[attrNum]);
	}

	// let value point to the global variable
	*value = attrValue;

	printf("END\n\n");
	return RC_OK;
} // getAttr

/**************************************************************************************
 * Function Name: setAttr
 *
 * Description:
 *		update the value of a given record of a given schema's attrNumth attribute to a given value
 *
 * Parameters:
 * 		Record *record: a record waiting to get update
 * 		Schema *schema: the schema of the given record
 * 		int attrNum: the ordinal of the attribute we want to update
 * 		Value *value: the value we want to update to
 *
 * Return:
 *		RC: success or error note
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-09  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 *		2015-04-10  Xiaolang Wang <xwang122@hawk.iit.edu>     correct fatal logical error
 **************************************************************************************/
RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
	printf("BEGIN-setAttr\n");
	// calculate the offset
	int attrOffset = 0;

	// check if passed in value's dt is the same with the attribute of schema
	// if dataType not the same, return RC_WRONG_DATATYPE
	if (value->dt != schema->dataTypes[attrNum]) {
		return RC_WRONG_DATATYPE;
	}

	// loop to add the length of each attribute before the target into offset
	int i;
	for (i = 0; i < attrNum; i++) {
		attrOffset += schema->typeLength[i];
	}

	// save value[attrNum] to the attrNumth attribute using memcpy
	if (value->dt == DT_STRING) {
		memcpy(record->data + attrOffset, value->v.stringV,
				schema->typeLength[attrNum]);
	} else if (value->dt == DT_BOOL) {
		memcpy(record->data + attrOffset, &(value->v.boolV),
				schema->typeLength[attrNum]);
	} else if (value->dt == DT_INT) {
		memcpy(record->data + attrOffset, &(value->v.floatV),
				schema->typeLength[attrNum]);
	} else if (value->dt == DT_FLOAT) {
		memcpy(record->data + attrOffset, &(value->v.intV),
				schema->typeLength[attrNum]);
	}

	printf("END\n\n");
	return RC_OK;
} // setAttr

/**************************************************************************************
 * Function Name: checkEqualsExpr
 *
 * Description:
 *		check if the expression is "attr = cons"
 *
 * Parameters:
 * 		Expr *setCond: the expression we want to check
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
 *		2015-04-20  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC checkEqualsExpr(Expr *cond) {
	RC rc = -99;

	//if ExprType is not operater return error message
	if (cond->type != EXPR_OP) {
		return RC_NOT_EQUALS_EXPR;
	}

	//then check if the Operator type is equals
	//if not, return error message
	if (cond->expr.op->type != OP_COMP_EQUAL) {
		return RC_NOT_EQUALS_EXPR;
	}

	//check if the left part of the operator is attribute
	//if not, return RC_LEFT_NOT_ATTR
	if (cond->expr.op->args[0]->type != EXPR_ATTRREF) {
		return RC_LEFT_NOT_ATTR;
	}

	//then check if the right part of the operator is a constant
	//if not, return RC_RIGHT_NOT_CONS
	if (cond->expr.op->args[1]->type != EXPR_CONST) {
		return RC_RIGHT_NOT_CONS;
	}

	return RC_OK;
}	//checkBinopExpr

/////////////////////////////// conditional updates ////////////////////////////////
/**************************************************************************************
 * Function Name: condUpdateRecord
 *
 * Description:
 *		Find all eligible records from the scan manager, then update them with new expression
 *		We only support for an update with expression "attribute = constant" by now
 *
 * Parameters:
 * 		RM_ScanHandle *scan: a scan handler to search for eligible records
 * 		Expr *setCond: a "attr = cons" expression to update
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
 *		2015-04-20  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC condUpdateRecord(RM_ScanHandle *scan, Expr *setCond) {
	printf("BEGIN-condUpdateRecord\n");
	RC rc;

	// check if the setCond is "attr = cons"
	rc = -99;
	rc = checkEqualsExpr(setCond);

	if (rc != RC_OK) {
		return rc;
	}

	//check if the setCond's attrRef is a valid number
	int numAttr = scan->rel->schema->numAttr;
	int attrRef = setCond->expr.op->args[0]->expr.attrRef;

	if (attrRef < 0 || attrRef >= numAttr) {
		return RC_INVALID_ATTRREF;
	}
	//check if the attribute type is the valid
	DataType dtAttr = scan->rel->schema->dataTypes[attrRef];
	DataType dtExpr = setCond->expr.op->args[1]->expr.cons->dt;

	if (dtAttr != dtExpr) {
		return RC_INVALID_EXPR_CONS_DATATYPE;
	}

	// search for the record
	// declare a Record and a Value variable
	Record *record;
	rc = createRecord(&record, scan->rel->schema);
	if (rc != RC_OK) {
		return rc;
	}

	//result equals to the constant in the set condition
	Value *result = (Value *) malloc(sizeof(Value));
	result = setCond->expr.op->args[1]->expr.cons;

	//use while loop to find each eligible record, pass in the record we just declared
	while ((rc = next(scan, record)) == RC_OK) {
		//after next, record should point to the record we want to find
		//set this record with the given expression
		rc = setAttr(record, scan->rel->schema, attrRef, result);
		if (rc != RC_OK) {
			return rc;
		}

		//update the slot with this set record
		rc = updateRecord(scan->rel, record);
		if (rc != RC_OK) {
			return rc;
		}

	}

	if (rc != RC_RM_NO_MORE_TUPLES) {
		return rc;
	}

	//free record and result;
	free(result);

	printf("END\n\n");
	return freeRecord(record);
} // conditionalUpdateRecord


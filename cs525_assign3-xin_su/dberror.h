#ifndef DBERROR_H
#define DBERROR_H

#include <stdio.h>

/* module wide constants */
#define PAGE_SIZE 4096  // page size: 4096 bytes

/* return code definitions */
typedef int RC; // return code: typedef to int

#define RC_OK 0 // ok
#define RC_FILE_NOT_FOUND 11 // file not found
#define RC_FILE_HANDLE_NOT_INIT 12 // file handler not inited
#define RC_WRITE_FILE_FAILED 13 // write failed
#define RC_NON_EXISTING_PAGE 14 // read non-existing page

/* BEGIN 2015-02-09, added by Xin Su <xsu11@hawk.iit.edu>, define more return codes for Storage Manager */
#define RC_SEEK_FILE_POSITION_ERROR 100	// seek the current position of the file failed
#define RC_SEEK_FILE_TAIL_ERROR 101 // seek file tail position failed
#define RC_CLOSE_FILE_FAILED 102 // close file failed
#define RC_REMOVE_FILE_FAILED 103 // remove file failed
#define RC_ENOUGH_PAGES 104 // enough pages, no need to append new pages
/* END 2015-02-09, added by Xin Su <xsu11@hawk.iit.edu> */

/* BEGIN 2015-02-10, added by Xiaolang Wang <xwang122@hawk.iit.edu>, define more return codes for Storage Manager */
#define RC_READ_FILE_FAILED 110 // read file failed
/* END 2015-02-10, added by Xiaolang Wang <xwang122@hawk.iit.edu> */

/* BEGIN 2015-03-14, added by Xin Su <xsu11@hawk.iit.edu>, define more return codes for Buffer Manager */
#define RC_NO_REMOVABLE_PAGE 120 // no removable page for replacement
#define RC_PAGELIST_NOT_INITIALIZED 121 // PageList is not initialized
#define RC_PAGE_NOT_FOUND 122 // page not found when searching the requested page in the PageList
#define RC_INVALID_NUMPAGES 123 // numPages of the Buffer Pool is invalid
#define RC_PAGE_FOUND 124 // search the requested page and found it in the BUffer Pool
#define RC_FLUSH_POOL_ERROR 125 // force flush pool meets error (some pages are in use)
#define RC_RS_NOT_IMPLEMENTED 126 // the requested replacement strategy is not implemented
/* END 2015-03-14, added by Xin Su <xsu11@hawk.iit.edu> */

/* BEGIN 2015-04-07, added by Xin Su <xsu11@hawk.iit.edu>, define more return codes for Record Manager */
#define RC_INVALID_TARGET_VALUE 130 // invalid expected value
#define RC_SCHEMA_NOT_CREATED 131 // schema not created
#define RC_OPEN_TABLE_FAILED 132 // open table failed
#define RC_WRONG_NEW_RESERVED_PAGE_NUM 133 // wrong new reserved page number
#define RC_GET_FIRST_FREE_PAGE_ERROR 134 // get first free page error
#define RC_TABLE_FILE_NO_ACCESS 135
#define RC_GET_IS_OCCUPIED_ERROR 136 // get isOccupied error
#define RC_CHECK_RECORD_LENGTH_ERROR 137 // check record length error
#define RC_PAGE_FULL_ERROR 138 // page is already full
#define RC_SET_TOTAL_PAGES_ERROR 139
#define RC_RID_OUT_OF_BOUND 140
#define RC_RID_IS_RESERVED_PAGE 141
#define RC_SLOT_EMPTY 142
#define RC_SLOT_ERROR 143
#define RC_MEMORY_COPY_ERROR 144
#define RC_WRONG_SCHEMA 145
#define RC_WRONG_DATATYPE 146
#define RC_TABLE_EXISTS 147
#define RC_SET_TOTAL_RECORDS_ERROR 148
#define RC_NO_RID 149
#define RC_NOT_EQUALS_EXPR 150
#define RC_LEFT_NOT_ATTR 151
#define RC_RIGHT_NOT_CONS 152
#define RC_INVALID_ATTRREF 153
#define RC_INVALID_EXPR_CONS_DATATYPE 154
/* END 2015-04-07, added by Xin Su <xsu11@hawk.iit.edu> */

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303

/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError(RC error);
extern char *errorMessage(RC error);

/* throw message */
#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

/* check the return code and exit if it is an error */
#define CHECK(code)							\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);

#endif

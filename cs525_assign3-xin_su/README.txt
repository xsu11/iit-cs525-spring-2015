~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Assignment-3: 

	The assignment is to accomplish a record manager of a database. All rights reserved.
	
* Extra credit:
	1. Free space management: 
		We use a very elegant way to manage free spaces in the table file. We create reserved page and use
		4000 bytes to store the free status for all record pages. Each record page's free status use only one bit to store 0 or 1.
		Therefore one reserved page can store 32000 record pages' free status. When all 3200 record page are full,
		we create a new reserved page in the end of the file to store the free status of additional 32000 new record pages.
		For details, Please see the Structure section below.
		
		we also create an additional test file for this implementation, test_assign3_2.c. In the test file, we create a table with
		a schema that has a length of record of 3500 bytes. Therefore one record page can only store one record. Then we insert
		35000 records into that table and see if wee create the second reserved page.
	
	2. Conditional Update:
		We implement the conditional update on the table. Please see the function description below for more details.
		
		We also create an additional test file, test_assign3_3.c. Please see the code file for more details.

* Created by:

	Xin Su <xsu11@hawk.iit.edu>

* Cooperated with:

	Chengnan Zhao <czhao18@hawk.iit.edu>,
	Jie Zhou <jzhou49@hawk.iit.edu>,
	Xiaolang Wang <xwang122@hawk.iit.edu>

* Included files:

	buffer_mgr.c
	buffer_mgr.h
	buffer_mgr_stat.c
	buffer_mgr_stat.h
	dberror.c
	dberror.h
	dt.h
	expr.c
	expr.h
	Makefile
	README.txt
	record_mgr.c
	record_mgr.h
	rm_serializer.c
	storage_mgr.c
	storage_mgr.h
	tables.h
	test.c
	test_assign1_1.c
	test_assign1_2.c
	test_assign2_1.c
	test_assign2_1_modified.c
	test_assign2_2.c
	test_assign3_1.c
	test_assign3_2.c
	test_assign3_3.c
	test_expr.c
	test_helper.h

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Assignment-3 Milestone:

	04/08/2015 - DEV Phase complete: Code and Unit Test
	04/18/2015 - SIT Phase complete: System Integration Test
	04/20/2015 - Delivery: Code and Documentation

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Installation instruction:

	0. Log in to server and cd to the directory of the code:

$ cd /home/class/spring-15/cs525/xsu11/assignment3
$ pwd
/home/class/spring-15/cs525/xsu11/assignment3
$ ls -l
-rwxr-xr-x. 1 xsu11 cs525ta 49117 Apr 19 14:28 buffer_mgr.c
-rwxr-xr-x. 1 xsu11 cs525ta  3397 Mar 21 00:34 buffer_mgr.h
-rwxr-xr-x. 1 xsu11 cs525ta  2181 Sep 16  2014 buffer_mgr_stat.c
-rwxr-xr-x. 1 xsu11 cs525ta   309 Sep 16  2014 buffer_mgr_stat.h
-rwxr-xr-x. 1 xsu11 cs525ta   597 Feb 15 11:49 dberror.c
-rwxr-xr-x. 1 xsu11 cs525ta  4281 Apr 23 14:27 dberror.h
-rwxr-xr-x. 1 xsu11 cs525ta   195 Sep 16  2014 dt.h
-rwxr-xr-x. 1 xsu11 cs525ta  3851 Apr 13 14:29 expr.c
-rwxr-xr-x. 1 xsu11 cs525ta  2895 Apr  1 13:27 expr.h
-rwxr-xr-x. 1 xsu11 cs525ta  2335 Apr 23 14:21 Makefile
-rwxr-xr-x. 1 xsu11 cs525ta 12179 Mar 28 23:53 README.txt
-rwxr-xr-x. 1 xsu11 cs525ta 96113 Apr 23 14:28 record_mgr.c
-rwxr-xr-x. 1 xsu11 cs525ta  1610 Apr 23 10:11 record_mgr.h
-rwxr-xr-x. 1 xsu11 cs525ta  7223 Apr 16 17:05 rm_serializer.c
-rwxr-xr-x. 1 xsu11 cs525ta 22592 Apr 17 12:49 storage_mgr.c
-rwxr-xr-x. 1 xsu11 cs525ta  1960 Feb 15 11:49 storage_mgr.h
-rwxr-xr-x. 1 xsu11 cs525ta  3561 Apr 16 16:41 tables.h
-rwxr-xr-x. 1 xsu11 cs525ta  2585 Feb 10 20:40 test_assign1_1.c
-rwxr-xr-x. 1 xsu11 cs525ta 10106 Feb 21 15:20 test_assign1_2.c
-rwxr-xr-x. 1 xsu11 cs525ta  7625 Sep 16  2014 test_assign2_1.c
-rwxr-xr-x. 1 xsu11 cs525ta  9864 Mar 28 22:20 test_assign2_1_modified.c
-rwxr-xr-x. 1 xsu11 cs525ta  4273 Mar 28 22:25 test_assign2_2.c
-rwxr-xr-x. 1 xsu11 cs525ta 17116 Nov 11 12:31 test_assign3_1.c
-rwxr-xr-x. 1 xsu11 cs525ta  5402 Apr 23 14:36 test_assign3_2.c
-rwxr-xr-x. 1 xsu11 cs525ta  4642 Apr 23 14:33 test_assign3_3.c
-rwxr-xr-x. 1 xsu11 cs525ta  3835 Nov 11 12:31 test_expr.c
-rwxr-xr-x. 1 xsu11 cs525ta  2467 Feb 10 20:40 test_helper.h

	1. Run “make clean” to clean the compiled files, executable files and log files if there is any:

$ make clean
rm -f test_assign1_1 test_assign1_2 test_assign2_1_modified test_assign2_2 test_assign3_1 test_assign3_2 test_assign3_3 test_expr *.o *.log
$ 

	2. Run “make” command to compile the code:

$ make
gcc -c storage_mgr.c
gcc -c dberror.c
gcc -c test_assign1_1.c
gcc -o test_assign1_1 storage_mgr.o dberror.o test_assign1_1.o
gcc -c test_assign1_2.c
gcc -o test_assign1_2 storage_mgr.o dberror.o test_assign1_2.o
gcc -c buffer_mgr_stat.c
gcc -c buffer_mgr.c
gcc -c test_assign2_1_modified.c
gcc -o test_assign2_1_modified storage_mgr.o buffer_mgr_stat.o buffer_mgr.o dberror.o test_assign2_1_modified.o -lpthread -std=gnu99 -I.
gcc -c test_assign2_2.c
gcc -o test_assign2_2 storage_mgr.o buffer_mgr_stat.o buffer_mgr.o dberror.o test_assign2_2.o -lpthread -std=gnu99 -I.
gcc -c expr.c
gcc -c record_mgr.c
gcc -c rm_serializer.c
gcc -c test_assign3_1.c
gcc -o test_assign3_1 buffer_mgr.o buffer_mgr_stat.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o test_assign3_1.o -lpthread -std=gnu99 -I.
gcc -c test_assign3_2.c
gcc -o test_assign3_2 buffer_mgr.o buffer_mgr_stat.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o test_assign3_2.o -lpthread -std=gnu99 -I.
gcc -c test_assign3_3.c
gcc -o test_assign3_3 buffer_mgr.o buffer_mgr_stat.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o test_assign3_3.o -lpthread -std=gnu99 -I.
gcc -c test_expr.c
gcc -o test_expr buffer_mgr.o buffer_mgr_stat.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o test_expr.o -lpthread -std=gnu99 -I.
$ ls -l
total 1036
-rwxr-xr-x. 1 xsu11 cs525ta  3397 Mar 21 00:34 buffer_mgr.h
-rw-r--r--. 1 xsu11 stud    16144 Apr 24 23:05 buffer_mgr.o
-rwxr-xr-x. 1 xsu11 cs525ta  2181 Sep 16  2014 buffer_mgr_stat.c
-rwxr-xr-x. 1 xsu11 cs525ta   309 Sep 16  2014 buffer_mgr_stat.h
-rw-r--r--. 1 xsu11 stud     4688 Apr 24 23:05 buffer_mgr_stat.o
-rwxr-xr-x. 1 xsu11 cs525ta   597 Feb 15 11:49 dberror.c
-rwxr-xr-x. 1 xsu11 cs525ta  4281 Apr 23 14:27 dberror.h
-rw-r--r--. 1 xsu11 stud     2296 Apr 24 23:05 dberror.o
-rwxr-xr-x. 1 xsu11 cs525ta   195 Sep 16  2014 dt.h
-rwxr-xr-x. 1 xsu11 cs525ta  3851 Apr 13 14:29 expr.c
-rwxr-xr-x. 1 xsu11 cs525ta  2895 Apr  1 13:27 expr.h
-rw-r--r--. 1 xsu11 stud     7664 Apr 24 23:05 expr.o
-rwxr-xr-x. 1 xsu11 cs525ta  2335 Apr 23 14:21 Makefile
-rwxr-xr-x. 1 xsu11 cs525ta 12179 Mar 28 23:53 README.txt
-rwxr-xr-x. 1 xsu11 cs525ta 96113 Apr 23 14:28 record_mgr.c
-rwxr-xr-x. 1 xsu11 cs525ta  1610 Apr 23 10:11 record_mgr.h
-rw-r--r--. 1 xsu11 stud    42120 Apr 24 23:05 record_mgr.o
-rwxr-xr-x. 1 xsu11 cs525ta  7223 Apr 16 17:05 rm_serializer.c
-rw-r--r--. 1 xsu11 stud    22840 Apr 24 23:05 rm_serializer.o
-rwxr-xr-x. 1 xsu11 cs525ta 22592 Apr 17 12:49 storage_mgr.c
-rwxr-xr-x. 1 xsu11 cs525ta  1960 Feb 15 11:49 storage_mgr.h
-rw-r--r--. 1 xsu11 stud     5408 Apr 24 23:05 storage_mgr.o
-rwxr-xr-x. 1 xsu11 cs525ta  3561 Apr 16 16:41 tables.h
-rwxr-xr-x. 1 xsu11 stud    14691 Apr 24 23:05 test_assign1_1
-rwxr-xr-x. 1 xsu11 cs525ta  2585 Feb 10 20:40 test_assign1_1.c
-rw-r--r--. 1 xsu11 stud     9872 Apr 24 23:05 test_assign1_1.o
-rwxr-xr-x. 1 xsu11 stud    22838 Apr 24 23:05 test_assign1_2
-rwxr-xr-x. 1 xsu11 cs525ta 10106 Feb 21 15:20 test_assign1_2.c
-rw-r--r--. 1 xsu11 stud    28688 Apr 24 23:05 test_assign1_2.o
-rwxr-xr-x. 1 xsu11 cs525ta  7625 Sep 16  2014 test_assign2_1.c
-rwxr-xr-x. 1 xsu11 stud    32946 Apr 24 23:05 test_assign2_1_modified
-rwxr-xr-x. 1 xsu11 cs525ta  9864 Mar 28 22:20 test_assign2_1_modified.c
-rw-r--r--. 1 xsu11 stud    25376 Apr 24 23:05 test_assign2_1_modified.o
-rwxr-xr-x. 1 xsu11 stud    27428 Apr 24 23:05 test_assign2_2
-rwxr-xr-x. 1 xsu11 cs525ta  4273 Mar 28 22:25 test_assign2_2.c
-rw-r--r--. 1 xsu11 stud    11976 Apr 24 23:05 test_assign2_2.o
-rwxr-xr-x. 1 xsu11 stud    88679 Apr 24 23:05 test_assign3_1
-rwxr-xr-x. 1 xsu11 cs525ta 17116 Nov 11 12:31 test_assign3_1.c
-rw-r--r--. 1 xsu11 stud    60808 Apr 24 23:05 test_assign3_1.o
-rwxr-xr-x. 1 xsu11 stud    68967 Apr 24 23:05 test_assign3_2
-rwxr-xr-x. 1 xsu11 cs525ta  5402 Apr 23 14:36 test_assign3_2.c
-rw-r--r--. 1 xsu11 stud    13952 Apr 24 23:05 test_assign3_2.o
-rwxr-xr-x. 1 xsu11 stud    67062 Apr 24 23:05 test_assign3_3
-rwxr-xr-x. 1 xsu11 cs525ta  4642 Apr 23 14:33 test_assign3_3.c
-rw-r--r--. 1 xsu11 stud     8888 Apr 24 23:05 test_assign3_3.o
-rwxr-xr-x. 1 xsu11 stud    69988 Apr 24 23:05 test_expr
-rwxr-xr-x. 1 xsu11 cs525ta  3835 Nov 11 12:31 test_expr.c
-rw-r--r--. 1 xsu11 stud    20800 Apr 24 23:05 test_expr.o
-rwxr-xr-x. 1 xsu11 cs525ta  2467 Feb 10 20:40 test_helper.h

	3. Run test_assign3_1 and redirect the output to file t1.log:

$ ./test_assign3_1 > t1.log
$ 

	4. View the result of the test_assign3_1 file:

$ vim -R t1.log
<t1.log content>
$

	5. Run test_assign3_2 and redirect the output to file t2.log:

$ ./test_assign3_2 > t2.log
$ 

	6. View the result of the test_assign3_2 file:

$ vim -R t2.log
<t2.log content>
$ 

	7. Run test_assign3_3 and redirect the output to file t3.log:

$ ./test_assign3_3 > t3.log
$ 

	8. View the result of the test_assign3_3 file:

$ vim -R t3.log
<t3.log content>
$ 

	9. Run test_expr and redirect the output to file t4.log:

$ ./test_expr > t4.log
$ 

	10. View the result of the test_expr file:

$ vim -R t4.log
<t3.log content>
$ 

	11. If you want to re-run the executable files, start from step 3. If you want to re-compile the files, start from step 1.
	
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Function Description:

	function name				description
	—————————————————————— 		———————————————————————————————————————————————————————————————————————————————————
	initRecordManager()			Initialize the Record Manager with a given entry to the buffer manager
	
	shutdownRecordManager()		Shut down the Record Manager
	
	initTableInfoPage()			Initialize the table info page

	createTable()				Create table file with given name and schema if it doesn't exist
								Then initialize the table info page and create the first reserved page

	openTable()					Open the table with the given name

	closeTable()				Close the passed in table

	deleteTable()				Delete the table with given name

	getRecordLength()			Get the length of a single record

	getTotalRecords()			Get the total records number stored in the table

	setTotalRecords()			Update the total records number to a user entered value

	getTotalPages()				Get the total number of pages stored in the table

	setTotalPages()				Update the total number of pages to a user entered value

	getPageMaxRecords()			Get the max number of records that can be stored in one page

	getNumTuples()				Get the total number of records stored in the table

	checkRID()					Check if the passed in RID is valid

	getReservedPageNum()		Get the reserved page number that records the passed in page

	isPageFree()				Check if the passed in page has empty slots
	
	getBit()					Get the value stored at a certain bit (TRUE/FALSE)

	setBit()					Set the value at a certain bit (TRUE/FALSE)

	createReservedPage()		Pin the reserved page and initialize it
	
	updateReservedPage()		Update the passed in reserved page

	getFirstFreePage()			Get the page number of the first page with free slots

	setFirstFreePage()			Update the page number of the first page on a reserved page

	getNextReservedPage()		Get the page number of next reserved page on the current reserved page

	setNextReservedPage()		Update the page number of next reserved page on the current reserved page
	
	getPrevReservedPage()		Get the page number of previous reserved page on the current reserved page

	setPrevReservedPage()		Update the page number of previous reserved page on the current reserved page

	searchFirstFreePage()		Search for the page number of the first page with free slots

	isPageInitialized()			Check if a certain page is initialized
	
	setPageInitialized()		Update a certain page's initialization status

	getCurrentRecords()			Get the number of records stored in a certain page

	setCurrentRecords()			Update the number of records stored in a certain page

	isSlotOccupied()			Check if a certain slot is occupied

	setSlotOccupied()			Update a certain slot's occupancy status

	checkRecordLength()			Check if the passed in record has a valid length

	initRecordPage()			Initialize a new record page

	getFreeSlotId()				Get the slot number of the first free slot on a certain page

	insertRecord()				Insert a record to the table
	
	deleteRecord()				Delete a record with given RID

	updateRecord()				Update a table slot with the passed in record

	getRecord()					Get a record from a certain table and certain RID
								If not found, return error code

	startScan()					Initialize a new scan manager with given table manager and given expression

	next()						Get the next eligible record in the table

	closeScan()					Unset the scan manager

	getRecordSize()				Get the size of the each record of given schema

	schemaToString()			Parse a schema into string

	createSchema()				Create a new schema with given values

	freeSchema()				Unset a schema
	
	createRecord()				Create an empty record of given schema

	freeRecord()				Unset a record

	getAttr()					Get the value of the attrNumth attribute for given record of given schema and 
								Store it at the first spot in an array of Values

	setAttr()					Update the value of the attrNumth attribute for given record to given value

	condUpdateRecord()			Find all eligible records from the scan manager, then update them with new expression
								We only support for an update with expression "attribute = constant" by now
	
	checkEqualsExpr()			Check if an expression is "attribute = constant"

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Structure Description:

	1. Page 0 stores all the table informationmation, including 
		1) record length - sizeof(int)
		2) number of total records - sizeof(int)
		3) number of total pages - sizeof(int)
		4) number of max records per page - sizeof(int)
		5) schema string
		
	2. Page 1 is a reserved page. It's used to manage free space information. The first 96 bytes are for page header, and the other 
		4000 bytes are used for free page information. We use only 1 bit for each page's free status: "1" indicates there EXIST 
		free slots on that page, "0" indicates there EXIST NO free slots on that page. Therefore each reserve page can store 32000 pages' 
		free status.
		
	3. After every 32000 record pages, we create a new reserved page. So the page numbers for reserved pages are 1, 32002, 64003...
		Each reserved page is linked with the previous and the next reserved page by information in the header. There is following 
		information in a reserved page header: 
			1) page number of the first page with free slots - sizeof(int)
			2) page number of the previous reserved page - sizeof(int)
			3) page number of the next reserved page - sizeof(int)
	4. Other pages are record pages. Information stored in a record page header includes: 
			1) page initialization status - sizeof(char)
			2) number of records stored on this page - sizeof(int)
			3) occupancy status for each slot - sizeof(char) for each slot, n * sizeof(char) totally
		Records are stored from bottom on each page by calculating the offset.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Additional error message:

	1. define more return codes for Storage Manager
	
		RC_SEEK_FILE_POSITION_ERROR 100	// seek the current position of the file failed
		RC_SEEK_FILE_TAIL_ERROR 101 // seek file tail position failed
		RC_CLOSE_FILE_FAILED 102 // close file failed
		RC_REMOVE_FILE_FAILED 103 // remove file failed
		RC_ENOUGH_PAGES 104 // enough pages, no need to append new pages
		RC_READ_FILE_FAILED 110 // read file failed

	2. define more return codes for Buffer Manager
	
		RC_NO_REMOVABLE_PAGE 120 // no removable page for replacement
		RC_PAGELIST_NOT_INITIALIZED 121 // PageList is not initialized
		RC_PAGE_NOT_FOUND 122 // page not found when searching the requested page in the PageList
		RC_INVALID_NUMPAGES 123 // numPages of the Buffer Pool is invalid
		RC_PAGE_FOUND 124 // search the requested page and found it in the BUffer Pool
		RC_FLUSH_POOL_ERROR 125 // force flush pool meets error (some pages are in use)
		RC_RS_NOT_IMPLEMENTED 126 // the requested replacement strategy is not implemented

	3. define more return codes for Record Manager
	
		RC_INVALID_TARGET_VALUE 130 // invalid expected value
		RC_SCHEMA_NOT_CREATED 131 // schema not created
		RC_OPEN_TABLE_FAILED 132 // open table failed
		RC_WRONG_NEW_RESERVED_PAGE_NUM 133 // wrong new reserved page number
		RC_GET_FIRST_FREE_PAGE_ERROR 134 // get first free page error
		RC_TABLE_FILE_NO_ACCESS 135
		RC_GET_IS_OCCUPIED_ERROR 136 // get isOccupied error
		RC_CHECK_RECORD_LENGTH_ERROR 137 // check record length error
		RC_PAGE_FULL_ERROR 138 // page is already full
		RC_SET_TOTAL_PAGES_ERROR 139
		RC_RID_OUT_OF_BOUND 140
		RC_RID_IS_RESERVED_PAGE 141
		RC_SLOT_EMPTY 142
		RC_SLOT_ERROR 143
		RC_MEMORY_COPY_ERROR 144
		RC_WRONG_SCHEMA 145
		RC_WRONG_DATATYPE 146
		RC_TABLE_EXISTS 147
		RC_SET_TOTAL_RECORDS_ERROR 148
		RC_NO_RID 149
		RC_NOT_EQUALS_EXPR 150
		RC_LEFT_NOT_ATTR 151
		RC_RIGHT_NOT_CONS 152
		RC_INVALID_ATTRREF 153
		RC_INVALID_EXPR_CONS_DATATYPE 154

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Test Cases:

	no	case content																	expectation			result
	--	------------------------------------------------------------------------------	------------------	------------------
	1	all functions in the given test case											Return OK			Return OK
	2	createTable(): when SCHEMA == NULL return error									Return error		Return error
	3	createTable(): when length of SCHEMA_STR is equal to 0 return error				Return error		Return error
	4	setTotalRecords(): input value < 0, return error								Return error		Return error
	5	setTotalPages(): input value < 0, return error									Return error		Return error
	6	checkRID(): input RID's page value < 0, return RC_RID_OUT_OF_BOUND				Return error		Return error
	7	checkRID(): if RID's page number is reserve page, return error					Return error		Return error
	8	setBit(): if input value is other than 0 or 1, return error						Return error		Return error
	9	setFirstFreePage: page number value is < 0, return error						Return error		Return error
	10	isPageFree(): page number didn't match reserve page number, return error		Return error		Return error
	11	setCurrentRecords(): input value is < 0, return error							Return error		Return error
	12	isSlotOccupied(): the page number is reserve page, return error					Return error		Return error
	13	setSlotOccupied(): the page number is not valid, return error					Return error		Return error
	14	setSlotOccupied(): the slot number is not valid, return error					Return error		Return error
	15	checkRecordLength(): input length and the getLength method is not the same		Return error		Return error
	16	getFreeSlotId(): input page number has no empty slot, return error				Return error		Return error
	17	insertRecord(): no empty space, need to create page first, return error			Return error		Return error
	18	deleteRecord(): input RID is not valid, return error							Return error		Return error
	19	updateRecord(): input Record's RID is not valid, return error					Return error		Return error
	20	getRecord(): input RID is not valid, return error								Return error		Return error
	21	next(): RID is NULL, return RC_NO_RID											Return error		Return error
	22	next(): not more tuples to read, return RC_RM_NO_MORE_TUPLES					Return error		Return error
	23	createSchema(): using wrong inputs, return error								Return error		Return error
	24	setAttr(): value's data type didn't match schema's, return error				Return error		Return error
	25	createRecord(): input schema is not valid, return error							Return error		Return error
	26  createReservedPage(): wrong reservedPageNum, return error						Return error		Return error
	27  updateReservedPage(): wrong reservedPageNum, return error						Return error		Return error
	28  updateReservedPage(): wrong PageNum, return error								Return error		Return error
	29  setFirstFreePage(): wrong reservedPageNum, return error							Return error		Return error
    30  getNextReservedPage(): wrong reservedPageNum, return error						Return error        Return error
	31  setNextReservedPage(): wrong reservedPageNum, return error						Return error		Return error
	32  getPrevReservedPage(): wrong reservedPageNum, return error						Return error		Return error
	
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Problems to be solved: 

	1. If we want to implement tombstonetone and TID, we would choose to still use 1 sizeof(char) to store the different status of a slot
		in the record page. We would use fore hex number to store four status:
			1) 0x00: slot is empty
			2) 0x01: slot is occupied
			3) 0x03: slot is deleted (tombstone)
			4) 0x04; slot is moved to another slot
		But the question raises when we want to move the slot. If we want to implement the TID, we have to store the new TID in the
		place where the record data is stored, suppose sizeof(int) for page number and sizeof(int) for slot number. However, when
		the record is very small (less then 2 * sizeof(int)), then we have a problem.
	
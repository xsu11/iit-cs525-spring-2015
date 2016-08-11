~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Assignment-2: 

	The assignment is to accomplish a buffer manager of a database. All rights reserved.

* Created by:

	Xin Su <xsu11@hawk.iit.edu>

* Cooperated with:

	Chengnan Zhao <czhao18@hawk.iit.edu>,
	Jie Zhou <jzhou49@hawk.iit.edu>,
	Xiaolang Wang <xwang122@hawk.iit.edu>

* Included files:

	Makefile
	README.txt
	buffer_mgr.c
	buffer_mgr.h
	buffer_mgr_stat.c
	buffer_mgr_stat.h
	dberror.c
	dberror.h
	dt.h
	storage_mgr.c
	storage_mgr.h
	test_assign1_1.c
	test_assign1_2.c
	test_assign2_1.c
	test_assign2_1_modified.c
	test_assign2_2.c
	test_helper.h

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Assignment-2 Milestone:

	03/16/2015 - DEV Phase complete: Coding and Unit Test
	03/26/2015 - SIT Phase complete: System integration test and destructive test
	03/28/2015 - Delivery: deliver code and documentation to server and Blackboard

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Installation instruction:

	0. Log in to server and cd to the directory of the code:

$ cd /home/class/spring-15/cs525/xsu11/assignment2
$ pwd
/home/class/spring-15/cs525/xsu11/assign2
$ ls -l
total 164
-rwxr-xr-x. 1 xsu11 stud 48604 Mar 28 16:45 buffer_mgr.c
-rwxr-xr-x. 1 xsu11 stud  3397 Mar 21 00:34 buffer_mgr.h
-rwxr-xr-x. 1 xsu11 stud  2181 Sep 16  2014 buffer_mgr_stat.c
-rwxr-xr-x. 1 xsu11 stud   309 Sep 16  2014 buffer_mgr_stat.h
-rwxr-xr-x. 1 xsu11 stud   642 Sep 16  2014 dberror.c
-rwxr-xr-x. 1 xsu11 stud  3036 Mar 28 16:04 dberror.h
-rwxr-xr-x. 1 xsu11 stud   195 Sep 16  2014 dt.h
-rwxr-xr-x. 1 xsu11 stud  1278 Mar 28 22:30 Makefile
-rwxr-xr-x. 1 xsu11 stud  8532 Feb 21 18:19 README.txt
-rwxr-xr-x. 1 xsu11 stud 22679 Feb 21 16:05 storage_mgr.c
-rwxr-xr-x. 1 xsu11 stud  1730 Sep 16  2014 storage_mgr.h
-rwxr-xr-x. 1 xsu11 stud  2585 Feb 10 20:40 test_assign1_1.c
-rwxr-xr-x. 1 xsu11 stud 10106 Feb 21 15:20 test_assign1_2.c
-rwxr-xr-x. 1 xsu11 stud  7625 Sep 16  2014 test_assign2_1.c
-rwxr-xr-x. 1 xsu11 stud  9864 Mar 28 22:20 test_assign2_1_modified.c
-rwxr-xr-x. 1 xsu11 stud  4273 Mar 28 22:25 test_assign2_2.c
-rwxr-xr-x. 1 xsu11 stud  2467 Sep 16  2014 test_helper.h
$ 

	1. Run “make clean” to clean the compiled files, executable files and log files if there is any:

$ make clean
rm -f test_assign1_1 test_assign1_2 test_assign2_1_modified test_assign2_2 *.o *.log
$ 

	2. Run “make” command to compile the code:

$ make
gcc -c dberror.c
gcc -c storage_mgr.c
gcc -c test_assign1_1.c
gcc -o test_assign1_1 dberror.o storage_mgr.o test_assign1_1.o
gcc -c test_assign1_2.c
gcc -o test_assign1_2 dberror.o storage_mgr.o test_assign1_2.o
gcc -c buffer_mgr_stat.c
gcc -c buffer_mgr.c
gcc -c test_assign2_1_modified.c
gcc -o test_assign2_1_modified dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o test_assign2_1_modified.o -lpthread
gcc -c test_assign2_2.c
gcc -o test_assign2_2 dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o test_assign2_2.o -lpthread
$ ls -l
total 388
-rwxr-xr-x. 1 xsu11 stud 48604 Mar 28 16:45 buffer_mgr.c
-rwxr-xr-x. 1 xsu11 stud  3397 Mar 21 00:34 buffer_mgr.h
-rw-r--r--. 1 xsu11 stud 15976 Mar 28 22:57 buffer_mgr.o
-rwxr-xr-x. 1 xsu11 stud  2181 Sep 16  2014 buffer_mgr_stat.c
-rwxr-xr-x. 1 xsu11 stud   309 Sep 16  2014 buffer_mgr_stat.h
-rw-r--r--. 1 xsu11 stud  4688 Mar 28 22:57 buffer_mgr_stat.o
-rwxr-xr-x. 1 xsu11 stud   642 Sep 16  2014 dberror.c
-rwxr-xr-x. 1 xsu11 stud  3036 Mar 28 16:04 dberror.h
-rw-r--r--. 1 xsu11 stud  2296 Mar 28 22:57 dberror.o
-rwxr-xr-x. 1 xsu11 stud   195 Sep 16  2014 dt.h
-rwxr-xr-x. 1 xsu11 stud  1278 Mar 28 22:30 Makefile
-rwxr-xr-x. 1 xsu11 stud  8532 Feb 21 18:19 README.txt
-rwxr-xr-x. 1 xsu11 stud 22679 Feb 21 16:05 storage_mgr.c
-rwxr-xr-x. 1 xsu11 stud  1730 Sep 16  2014 storage_mgr.h
-rw-r--r--. 1 xsu11 stud  5408 Mar 28 22:57 storage_mgr.o
-rwxr-xr-x. 1 xsu11 stud 14691 Mar 28 22:57 test_assign1_1
-rwxr-xr-x. 1 xsu11 stud  2585 Feb 10 20:40 test_assign1_1.c
-rw-r--r--. 1 xsu11 stud  9872 Mar 28 22:57 test_assign1_1.o
-rwxr-xr-x. 1 xsu11 stud 22838 Mar 28 22:57 test_assign1_2
-rwxr-xr-x. 1 xsu11 stud 10106 Feb 21 15:20 test_assign1_2.c
-rw-r--r--. 1 xsu11 stud 28688 Mar 28 22:57 test_assign1_2.o
-rwxr-xr-x. 1 xsu11 stud  7625 Sep 16  2014 test_assign2_1.c
-rwxr-xr-x. 1 xsu11 stud 32914 Mar 28 22:57 test_assign2_1_modified
-rwxr-xr-x. 1 xsu11 stud  9864 Mar 28 22:20 test_assign2_1_modified.c
-rw-r--r--. 1 xsu11 stud 25376 Mar 28 22:57 test_assign2_1_modified.o
-rwxr-xr-x. 1 xsu11 stud 27412 Mar 28 22:57 test_assign2_2
-rwxr-xr-x. 1 xsu11 stud  4273 Mar 28 22:25 test_assign2_2.c
-rw-r--r--. 1 xsu11 stud 11976 Mar 28 22:57 test_assign2_2.o
-rwxr-xr-x. 1 xsu11 stud  2467 Sep 16  2014 test_helper.h
$ 

	3. Run test_assign2_1_modified and redirect the output to file t1.log:

$ ./test_assign2_1_modified > t1.log
$ 

	4. View the result of the test_assign1_1 file:

$ vim -R t1.log
<t1.log content>
$

	5. Run test_assign2_2 and redirect the output to file t2.log:

$ ./test_assign2_2 > t2.log
$ 

	6. View the result of the test_assign2_2 file:

$ vim -R t2.log
<t2.log content>
$ 

	7. If you want to re-run the executable files, start from step 3. If you want to re-compile the files, start from step 1.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Function Description:

	function name		description
	——————————————————————  ————————————————————————————————————————————————————————————————
	searchPage				Search the requested page in the Buffer Pool,
							if found, load the requested page into the BM_pageHandle and return RC_OK
							else, return error code
	
	appendPage()			Read the requested page from the disk and append it to the tail of the PageList
	
	replacePage()			Replace the current page with the requested page read from the disk
	
	FIFO()					FIFO replacement strategy
	
	LRU()					LRU replacement strategy
	
	CLOCK()					CLOCK replacement strategy
	
	initPageList()			Initialize the PageList to store pages in the Buffer Pool
	
	initBufferPool()		Initialize the Buffer Pool
	
	shutdownBufferPool()	Shut down the Buffer Pool
	
	forceFlushPool()		Write back the data in all dirty pages in the Buffer Pool
	
	pinPage()				Pin the page with the requested pageNum in the BUffer Pool
							If the page is not in the Buffer Pool, load it from the file to the Buffer Pool
	
	markDirty()				Mark the requested page as dirty
	
	unpinPage()				Unpin a page
	
	forcePage()				Write the requested page back to the page file on disk
	
	getFrameContents()		Returns an array of PageNumbers (of size numPages)
							An empty page frame is represented using the constant NO_PAGE
	
	getDirtyFlags()			Returns an array of bools (of size numPages)
							Empty page frames are considered as clean
	
	getFixCounts()			Returns an array of ints (of size numPages)
	
	getNumReadIO()			Returns the number of pages that have been read from disk
							since the Buffer Pool has been initialized
	
	getNumWriteIO()			Return the number of pages written to the page file
							since the Buffer Pool has been initialized

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Structure Description:

	1. Please refer to the pdf file with the code files.
	2. In this pdf file, I clearly draw a picture of the data structure used in this Buffer Manager.
	3. Because of the different data structure I use to implement FIFO, LRU and CLOCK, which is simpler on implementation and saving memory,
		the test case in test_assign2_1.c cannot be used on our code. Then we modified to generate our own test cases which is in the file
		test_assign2_1_modified.c. It shows all modified parts in that file. Also we build additional test cases on CLOCK in file
		test_assign2_2.c.
	4. For more deatils on our modification of the test cases and implementation of our data structure. I will send emails to clarify.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Test Cases:

	no	case content																	expectation				result		
	——  ——————————————————————————————————————————————————————————————————————————————  ——————————————————  ——————————————————
	1	normal processes of all	function												Return OK			Return OK

	2	searchPage(): when the PageList's size < 0, returns error						Return error		Return error

	3	searchPage(): when the PageList's size < 0, returns PAGE_NOT_FOUND				Return error		Return error

	4	calling openPageFile() of the Storage Manager meets error						Return error		Not test

	5	calling closePageFile() of the Storage Manager meets error						Return error		Not test

	6	calling readBlock() of the Storage Manager meets error							Return error		Not test

	7	calling writeBlock() of the Storage Manager meets error							Return error		Not test

	8	calling ensureCapacity() of the Storage Manager meets error						Return error		Not test

	9	FIFO(): when calling searchPage() meets unexpected error, return error			Return error		Return error

	10	FIFO(): calling searchPage() found the requested page, return RC_PAGE_FOUND		Return error		Return error

	11	FIFO(): when calling appendPage() meets unexpected error, return error			Return error		Return error

	12	FIFO(): when there is no removable page, return RC_NO_REMOVABLE_PAGE			Return error		Return error

	13	FIFO(): when calling replacePage() meets unexpected error, return error			Return error		Return error

	14	LRU(): when calling FIFO() meets unexpected error, return error					Return error		Return error

	15	CLOCK(): when calling searchPage() meets unexpected error, return error			Return error		Return error

	16	CLOCK(): calling searchPage() found the requested page, return RC_PAGE_FOUND	Return error		Return error

	17	CLOCK(): when calling appendPage() meets unexpected error, return error			Return error		Return error

	18	CLOCK(): when there is no removable page, return RC_NO_REMOVABLE_PAGE			Return error		Return error

	19	CLOCK(): when calling replacePage() meets unexpected error, return error		Return error		Return error

	20	initBufferPool(): if the numPages <= 0, return RC_INVALID_NUMPAGES				Return error		Return error

	21	shutdownBufferPool(): if page unwritable to the disk, return error				Return error		Return error

	22	forceFlushPool(): if page unwritable to the disk, return RC_FLUSH_POOL_ERROR	Return error		Return error

	23	pinPage(): when calling replacement strategies meets error, return error		Return error		Return error

	24	markDirty(): when the requested page does not exist, return error				Return error		Return error

	25	unpinPage(): when the requested page does not exist, return error				Return error		Return error

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Problems to be solved

	1. Not enough time to test thread safe, also lack of instances. 
	2. When pining a non-existent page from the file, it should return error.
		But now it create a new page in the file and load it into the Buffer Pool
	3. I implement other two flags: numReadIO and numWriteIO for each PageFrame.
		For now, the usage of this two flags has not received enought tests yet.
	4. There are other two more replacement strategy to be implemented: LFU and LRU-K

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

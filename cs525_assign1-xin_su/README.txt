~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Assignment-1: 

	The assignment is to accomplish a storage manager of a database. All rights reserved.

* Created by:

	Xin Su <xsu11@hawk.iit.edu>

* Cooperated with:

	Chengnan Zhao <czhao18@hawk.iit.edu>,
	Jie Zhou <jzhou49@hawk.iit.edu>,
	Xiaolang Wang <xwang122@hawk.iit.edu>

* Included files:

	Makefile
	README.txt
	dberror.c
	dberror.h
	storage_mgr.c
	storage_mgr.h
	test_assign1_1.c
	test_assign1_2.c
	test_helper.h

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Assignment-1 Milestone:

	02/12/2015 - DEV Phase complete: Coding and Unit Test
	02/20/2015 - SIT Phase complete: System integration test and destructive test
	02/21/2015 - Delivery: deliver code and documentation to server and Blackboard

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Installation instruction:

	0. Log in to server and cd to the directory of the code:

$ cd /home/class/spring-15/cs525/xsu11/assign1
$ pwd
/home/class/spring-15/cs525/xsu11/assign1
$ ls -l
total 64
-rw-r--r--. 1 xsu11 stud   597 Feb 15 11:49 dberror.c
-rw-r--r--. 1 xsu11 stud  2311 Feb 17 10:39 dberror.h
-rw-r--r--. 1 xsu11 stud   574 Feb 21 11:25 Makefile
-rw-r--r--. 1 xsu11 stud   773 Feb 21 15:28 README.txt
-rw-r--r--. 1 xsu11 stud 22678 Feb 21 15:32 storage_mgr.c
-rw-r--r--. 1 xsu11 stud  1960 Feb 15 11:49 storage_mgr.h
-rw-r--r--. 1 xsu11 stud  2585 Feb 10 20:40 test_assign1_1.c
-rw-r--r--. 1 xsu11 stud 10106 Feb 21 15:20 test_assign1_2.c
-rw-r--r--. 1 xsu11 stud  2467 Feb 10 20:40 test_helper.h
$ 

	1. Run “make clean” to clean the compiled files, executable files and log files

$ make clean
rm -f test_assign1_1 test_assign1_2 *.o *.log
$ 

	2. Run “make” command to compile the code:

$ make
gcc -c dberror.c
gcc -c storage_mgr.c
gcc -c test_assign1_1.c
gcc -o test_assign1_1 dberror.o storage_mgr.o test_assign1_1.o
gcc -c test_assign1_2.c
gcc -o test_assign1_2 dberror.o storage_mgr.o test_assign1_2.o
$ ls -l
total 160
-rw-r--r--. 1 xsu11 stud   597 Feb 15 11:49 dberror.c
-rw-r--r--. 1 xsu11 stud  2311 Feb 17 10:39 dberror.h
-rw-r--r--. 1 xsu11 stud  2296 Feb 21 15:33 dberror.o
-rw-r--r--. 1 xsu11 stud   574 Feb 21 11:25 Makefile
-rw-r--r--. 1 xsu11 stud   773 Feb 21 15:28 README.txt
-rw-r--r--. 1 xsu11 stud 22678 Feb 21 15:32 storage_mgr.c
-rw-r--r--. 1 xsu11 stud  1960 Feb 15 11:49 storage_mgr.h
-rw-r--r--. 1 xsu11 stud  5408 Feb 21 15:33 storage_mgr.o
-rwxr-xr-x. 1 xsu11 stud 14691 Feb 21 15:33 test_assign1_1
-rw-r--r--. 1 xsu11 stud  2585 Feb 10 20:40 test_assign1_1.c
-rw-r--r--. 1 xsu11 stud  9872 Feb 21 15:33 test_assign1_1.o
-rwxr-xr-x. 1 xsu11 stud 22838 Feb 21 15:33 test_assign1_2
-rw-r--r--. 1 xsu11 stud 10106 Feb 21 15:20 test_assign1_2.c
-rw-r--r--. 1 xsu11 stud 28688 Feb 21 15:33 test_assign1_2.o
-rw-r--r--. 1 xsu11 stud  2467 Feb 10 20:40 test_helper.h
$ 

	3. Run test_assign1_1 and redirect the output to file t1.log:

$ ./test_assign1_1 > t1.log
$ 

	4. View the result of the test_assign1_1 file:

$ vim -R t1.log
<t1.log content>
$

	5. Run test_assign1_2 and redirect the output to file t2.log:

$ ./test_assign1_1 > t2.log
$ 

	6. View the result of the test_assign1_2 file:

$ vim -R t2.log
<t2.log content>
$ 

	7. If you want to re-run the executable files, start from step 3. If you want to re-compile the files, start from step 1.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Function Description:

	function name		description
	——————————————————————  ————————————————————————————————————————————————————————————————
	initStorageManager()	Initialize the StorageManager.

	createPageFile()	Create a page file and write content.

	openPageFile()		Open the created pageFile and store relative information into fHandle.

	closePageFile()		Close the file.

	destroyPageFile()	Destroy (delete) a page file.

	readBlock()		The method reads the pageNum-th block from a file and
				stores its content in the memory pointed to by the memPage page handle.
				If the file has less than pageNum pages, the method should return RC_READ_NON_EXISTING_PAGE.

	getBlockPos()		Return the current page position in a file.

	readFirstBlock()	Return the first page position of a file.

	readPreviousBlock()	Read page from the previous block.

	readCurrentBlock()	Read page from the Current block.

	readNextBlock()		Read one page from the Next block.

	readLastBlock()		Read page from the last block.

	writeBlock()		Write one page BACK to the file (disk) start from absolute position.

	writeCurrentBlock()	Write one page BACK to the file (disk) of current position.

	appendEmptyBlock()	Write an empty page to the file (disk) by appending to the end.

	ensureCapacity()	If the file has less than numberOfPages pages then increase the size to numberOfPages.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Test Cases:

	no.	case content			expectation		result		
	——————  ——————————————————————————————  ——————————————————————  ——————————————————————
	1	normal processes of all		Return OK		Return OK
		functions.

	2	readFirstBlock():		Return error		Return error
		when curPos is 0, it returns
		error.

	3	readNextBlock():		Return error		Return error
		when curPos = totalNumPages,
		it returns error.

	4	writeCurrentBlock():		Return error		Return error
		when curPos < 0, it returns
		error.

	5	writeCurrentBlock():		Return error		Return error
		when curPos >= totalNumPages,
		it returns error.

	6	ensureCapasity():		Return error		Return error
		when numberOfPages is less
		then totalNumPages, it return
		error.

	7	fopen() createPageFile():	Return error		untested
		when the file is already open,
		it returns error.

	8	fopen() createPageFile():	Return error		untested
		when the file exists, it
		returns error.

	9	fopen() in openPageFile():	Return error		untested
		when the file is already open,
		it returns error.

	10	fopen() in openPageFile():	Return OK		untested
		when the file is already open,
		it open the existed file.

	11	fopen() in openPageFile():	Return OK		untested
		when the file is already
		deleted, it returns error.

	12	fclose():			Return error		untested
		when the file is already
		closed, it returns error.

	13	fclose():			Return error		untested
		when the file is already
		deleted, it returns error.

	14	fseek():			Return error		Return error
		when the file is already
		closed, it returns error.

	15	fseek():			Return error		Return error
		when the file is already
		deleted, it returns error.

	16	ftell():			Return error		Return error
		when the file is already
		closed, it returns error.

	17	ftell():			Return error		Return error
		when the file is already
		deleted, it returns error.

	18	fread():			Return error		Return error
		when the file is already
		closed, it returns error.

	19	fread():			Return error		Return error
		when the file is already
		deleted, it returns error.

	20	fwrite():			Return error		Return error
		when the file is already
		closed, it returns error.

	21	fwrite():			Return error		Return error
		when the file is already
		deleted, it returns error.

	22	remove():			Return error		Return error
		when the file is already
		closed, it returns error.

	21	remove():			Return error		Return error
		when the file is already
		deleted, it returns error.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Problems to be solved

	1. what should the code do to clean the unfinished write of the empty block when fwrite() meets error?
	2. what if only part of the stream is written to the file and fwrite() does not return error?
	3. whether it needs to backup before write the block over the original file?

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

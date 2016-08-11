~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Assignment-4: 

	In this assignment we are implementing a B+-tree index. 

* Created by:

	Xin Su <xsu11@hawk.iit.edu>

* Cooperated with:

	Chengnan Zhao <czhao18@hawk.iit.edu>,
	Jie Zhou <jzhou49@hawk.iit.edu>,
	Xiaolang Wang <xwang122@hawk.iit.edu>

* Included files:

	Makefile 
	buffer_mgr.c 
	buffer_mgr.h 
	buffer_mgr_stat.c 
	buffer_mgr_stat.h 
	btree_mgr.c 
	btree_mgr.h 
	dberror.c 
	dberror.h 
	dt.h 
	expr.c 
	expr.h 
	record_mgr.c 
	record_mgr.h 
	rm_serializer.c 
	storage_mgr.h 
	tables.h 
	test_assign4_1.c 
	test_expr.c 
	test_helper.h

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Assignment-4 Milestone:

	04/28/2015 - DEV Phase complete: Coding and Unit Test
	04/30/2015 - SIT Phase complete: System integration test and destructive test
	05/05/2015 - Delivery: deliver code and documentation to server and Blackboard

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Function Description:

	function name		description
	——————————————————————  ————————————————————————————————————————————————————————————————
	createBtree()     Create a B+ Tree with given name, DataType and number of 				elements in a node

	openBtree()	      Open a B+ Tree with given name

	closeBtree()	Close given B+ Tree

	deleted()		Delete B+ Tree with given name
	
	getPageInfo()	get the page information
	
	setPageInfo()	set the page information	
	
	getNumNodes()	get the total number of nodes
	
	setNumNodes()     set the total number of nodes	
	
	getNumEntries()	get the total number of entries
	
	setNumEntries()	set the total number of entries
	
	getKeyType()	get the key type
	
	setKeyType()      set the key type
	
	getTypeLength()	get the length of type
	
	setTypeLength()	set the length of type
	
	getN()		get the numbers of N
	
	setN()		set the numbers of N
	
	getRootPage()	get the current root page
	
	setRootPage()	set the root page
	
	createNodePage()  create node page
	
	getNodeState()    return the node state, leaf or non-leaf

	setNodeState	set the node state, leaf or non-leaf

	getCurrentKeys	get current keys

	setCurrentKeys    set current keys

	getParent		get parent node

	setParent		set parent node

	getLeftSibling    get left sibling

	setLeftSibling    set left sibling

	getRightSibling   get right sibling

	setRightSibling   set right sibling

	getKey 		get the key

	setKey		set the key

	getKeyPointer     get the key pointer

	setKeyPointer	set the key pointer

	findKey		find the key

	insertKey    	insert the key

	deleteKey		delete the key

	nextEntry		find the next entry

	combineNode		combine the nodes

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


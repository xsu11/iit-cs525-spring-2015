#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "record_mgr.h"
#include "storage_mgr.h"
#include "tables.h"
#include "btree_mgr.h"

#define INDEX_INFO_PAGE 0

// Global Veriables
BM_BufferPool *BM = NULL;
BTreeHandle *BTREE_HANDLE = NULL;
// BT_ScanHandle *SCAN_HANDLE = NULL;

//Prototypes
RC initIndexManager(void *mgmtData);
RC shutdownIndexManager();

RC createBtree(char *idxId, DataType keyType, int n);
RC openBtree(BTreeHandle **tree, char *idxId);
RC closeBtree(BTreeHandle *tree);
RC deleteBtree(char *idxId);

RC getPageInfo(BTreeHandle *tree, PageNumber pageNum, int pos, int *result);
RC setPageInfo(BTreeHandle *tree, int value, PageNumber pageNum, int pos);

RC getNumNodes(BTreeHandle *tree, int *result);
RC setNumNodes(BTreeHandle *tree, int value);

RC getNumEntries(BTreeHandle *tree, int *result);
RC setNumEntries(BTreeHandle *tree, int value);

RC getKeyType(BTreeHandle *tree, DataType *result);
RC setKeyType(BTreeHandle *tree, DataType value);

RC getTypeLength(BTreeHandle *tree, int * result);
RC setTypeLength(BTreeHandle *tree, int value);

RC getN(BTreeHandle *tree, int *result);
RC setN(BTreeHandle *tree, int value);

RC getRootPage(BTreeHandle *tree, PageNumber *result);
RC setRootPage(BTreeHandle *tree, int value);

RC createNodePage(BTreeHandle *tree, PageNumber pageNum, int nodeState,
		int parent, int leftSibling, int rightSibling);

RC getNodeState(BTreeHandle *tree, PageNumber pageNum, int *result);
RC setNodeState(BTreeHandle *tree, int value, PageNumber pageNum);

RC getCurrentKeys(BTreeHandle *tree, PageNumber pageNum, int *result);
RC setCurrentKeys(BTreeHandle *tree, int value, PageNumber pageNum);

RC getParent(BTreeHandle *tree, PageNumber pageNum, PageNumber *result);
RC setParent(BTreeHandle *tree, int value, PageNumber pageNum);

RC getLeftSibling(BTreeHandle *tree, PageNumber pageNum, PageNumber *result);
RC setLeftSibling(BTreeHandle *tree, int value, PageNumber pageNum);

RC getRightSibling(BTreeHandle *tree, PageNumber pageNum, PageNumber *result);
RC setRightSibling(BTreeHandle *tree, int value, PageNumber pageNum);

RC getKey(BTreeHandle *tree, int keyNum, PageNumber pageNum, PageNumber *result);
RC setKey(BTreeHandle *tree, int keyNum, int value, PageNumber pageNum);

RC getKeyPointer(BTreeHandle *tree, int keyPointerNum, PageNumber pageNum,
		int *result);
RC setKeyPointer(BTreeHandle *tree, int keyPointerNum, PageNumber keyPage,
		int keySlot, PageNumber pageNum);

RC findKey(BTreeHandle *tree, Value *key, RID *result);
RC insertKey(BTreeHandle *tree, Value *key, RID rid);
RC deleteKey(BTreeHandle *tree, Value *key);

RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle);
RC nextEntry(BT_ScanHandle *handle, RID *result);
RC closeTreeScan(BT_ScanHandle *handle);

RC combineNode();
RC insertInternalNode();

// char *printTree(BTreeHandle *tree);

/**************************************************************************************
 * Function Name: initInfexManager
 *
 * Description:
 *		Create an index manager with given buffer manager entry
 *
 * Parameters:
 * 		void * mgmtData: buffer manager entry
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC initIndexManager(void *mgmtData) {
	RC rc;

	// init BUffer Manager
	// init parameters for Buffer Manager
	BM = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));

	ReplacementStrategy strategy = RS_LRU;
	int numPages = 10;

	// Call initBufferPool() to init Buffer Manager (10, LRU)
	// Shutdown it in shutdownRrecordManager()
	rc = -99;
	rc = initBufferPool(BM, "", numPages, strategy, NULL);

	if (rc != RC_OK) {
		return rc;
	}

	// Allocate memory for global tree handle and scan handle
	BTREE_HANDLE = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	// SCAN_HANDLE = (BT_ScanHandle *) malloc(sizeof(BT_ScanHandle));

	return RC_OK;
}

/**************************************************************************************
 * Function Name: shutdownIndexManager
 *
 * Description:
 *		close and free the index manager
 *
 * Parameters:
 * 		N/A
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC shutdownIndexManager() {
	RC rc;

	// Shutdown Buffer Manager
	rc = -99;
	rc = shutdownBufferPool(BM);

	if (rc != RC_OK) {
		return rc;
	}

	free(BM);
	BM = NULL;

	free(BTREE_HANDLE);
	BTREE_HANDLE = NULL;

	// free(SCAN_HANDLE);
	// SCAN_HANDLE = NULL;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: createBtree
 *
 * Description:
 *		Create a B+ Tree with given name, DataType and number of elements in a node
 *
 * Parameters:
 * 		char *idxId: name of the B+ Tree
 * 		DataType keyType: data type of the tree
 * 		int n: number of elements in one node
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC createBtree(char *idxId, DataType keyType, int n) {
	printf("test - createBtree begin\n");
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// Check if the file exists
	// rc = -99;
	// rc = access(idxId, F_OK);

	// if the file does not exist (rc == -1), create it by calling createPageFile(),
	// else if the file exists (rc == 0), then return error code to report it
	// else, some error happens, return rc
	// if (rc == -1) {
		rc = createPageFile(idxId);

		// if (rc != RC_OK) {
		// 	return rc;
		//}
	// } else if (rc == 0) {
	// 	return RC_TABLE_EXISTS;
	// } else {
	// 	return rc;
	// }

	// pin page 0, and write NUM_NODES, NUM_ENTRIES, KEY_TYPE = 0 (4 bytes each)
	rc = -99;
	rc = pinPage(BM, page, INDEX_INFO_PAGE);

	if (rc != RC_OK) {
		return rc;
	}

	int typeLength;
	switch (keyType) {
	case DT_INT:
		typeLength = sizeof(int);
		break;
	case DT_FLOAT:
		typeLength = sizeof(float);
		break;
	case DT_BOOL:
		typeLength = sizeof(bool);
		break;
	case DT_STRING:
		typeLength = 10 * sizeof(char);
	}

	// Store the following info:
	// ip[0] # OF NODES - sizeof(int)
	// ip[1] # OF ENTRIES - sizeof(int)
	// ip[2] KEY_TYPE - sizeof(int)
	// ip[3] TYPE_LENGTH - sizeof(int)
	// ip[4] MAX_KEY PER NODE - sizeof(int)
	// ip[5] ROOT PAGE - sizeof(int)
	// ip[6] TOTAL PAGES - sizeof(int)
	int *ip = (int *) page->data;
	ip[0] = 0;
	ip[1] = 0;
	ip[2] = keyType;
	ip[3] = typeLength;
	ip[4] = n;
	ip[5] = 0;
	ip[6] = 1;

	// unpin page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	BTREE_HANDLE->keyType = keyType;

	free(page);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: openBtree
 *
 * Description:
 *		Open a B+ Tree with given name
 *
 * Parameters:
 * 		BTreeHandle **tree: A pointer to an array of B+ Tree, we will only use the first spot
 * 		char *idxId: Name of the B+ Tree
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC openBtree(BTreeHandle **tree, char *idxId) {
	// if the file has no access, then return error code to report it
	if (access(idxId, R_OK) != 0 || access(idxId, W_OK) != 0) {
		return RC_TABLE_FILE_NO_ACCESS;
	}

	// Set the name of the file to the Buffer Manager
	BM->pageFile = idxId;

	*tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	(*tree)->idxId = (char *) calloc(strlen(idxId) + 1, sizeof(char));

	// Set the tree structure
	// Store the entry of the Buffer Manager here in the tree structure
	strcpy((*tree)->idxId, idxId);

	DataType keyType;

	RC rc = getKeyType(*tree, &keyType);

	if (rc != RC_OK) {
		return rc;
	}

	(*tree)->keyType = keyType;
	(*tree)->mgmtData = BM;

	BTREE_HANDLE->idxId = idxId;
	BTREE_HANDLE->mgmtData = BM;

	// Store the entry of the tree in the scan_handle structure
	// SCAN_HANDLE->tree = *tree;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: closeBtree
 *
 * Description:
 *		Close given B+ Tree
 *
 * Parameters:
 * 		BTreeHandle *tree: Entry to the B+ Tree we about to close
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC closeBtree(BTreeHandle *tree) {
	RC rc;

	// Write all updated content in the Buffer Pool
	rc = -99;
	rc = forceFlushPool(BM);

	if (rc != RC_OK) {
		return rc;
	}

	// Unset the file name in Buffer Manager
	BM->pageFile = NULL;

	free(tree->idxId);

	// Unset the tree info
	tree->idxId = NULL;
	// tree->keyType = NULL;
	tree->mgmtData = NULL;

	free(tree);
	tree = NULL;

	// unset the BTREE_HANDLE structure
	BTREE_HANDLE->idxId = NULL;
	// BTREE_HANDLE->keyType = NULL;
	BTREE_HANDLE->mgmtData = NULL;

	// unset the SCAN_HANDLE structure
	// SCAN_HANDLE->tree = NULL;
	// SCAN_HANDLE->mgmtData = NULL;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: deleteBtree
 *
 * Description:
 *		Delete B+ Tree with given name
 *
 * Parameters:
 * 		char *idxId: name of the B+ Tree we about to delete
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC deleteBtree(char *idxId) {
	RC rc;

	rc = destroyPageFile(idxId);

	if (rc != RC_OK) {
		return rc;
	}

	return RC_OK;
}

/********************************************************************************************/
RC getPageInfo(BTreeHandle *tree, PageNumber pageNum, int pos, int *result) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0, page 0 is the information page of b+ tree
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	// get the first # of page 0, is # of nodes
	int *ip = (int *) page->data;
	*result = ip[pos];

	// unpin page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	return RC_OK;
}

RC setPageInfo(BTreeHandle *tree, int value, PageNumber pageNum, int pos) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// pin page 0, page 0 is the information page of b+ tree
	rc = -99;
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	// set the first # of page value
	int *ip = (int *) page->data;
	ip[pos] = value;

	// unpin page 0
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	return RC_OK;
}

/********************************************************************************************/
RC getNumNodes(BTreeHandle *tree, int *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 0, result);
}

RC setNumNodes(BTreeHandle *tree, int value) {
	return setPageInfo(tree, value, INDEX_INFO_PAGE, 0);
}

RC getNumEntries(BTreeHandle *tree, int *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 1, result);
}

RC setNumEntries(BTreeHandle *tree, int value) {
	return setPageInfo(tree, value, INDEX_INFO_PAGE, 1);
}

RC getKeyType(BTreeHandle *tree, DataType *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 2, result);
}

RC setKeyType(BTreeHandle *tree, DataType value) {
	return setPageInfo(tree, value, INDEX_INFO_PAGE, 2);
}

RC getTypeLength(BTreeHandle *tree, int *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 3, result);
}

RC setTypeLength(BTreeHandle *tree, int value) {
	return setPageInfo(tree, value, INDEX_INFO_PAGE, 3);
}

RC getN(BTreeHandle *tree, int *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 4, result);
}

RC setN(BTreeHandle *tree, int value) {
	return setPageInfo(tree, value, INDEX_INFO_PAGE, 4);
}

RC getRootPage(BTreeHandle *tree, PageNumber *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 5, result);
}
RC setRootPage(BTreeHandle *tree, PageNumber value) {
	return setPageInfo(tree, (int) value, INDEX_INFO_PAGE, 5);
}

RC getTotalNodePages(BTreeHandle *tree, int *result) {
	return getPageInfo(tree, INDEX_INFO_PAGE, 6, result);
}
RC setTotalNodePages(BTreeHandle *tree, int value) {
	return setPageInfo(tree, (int) value, INDEX_INFO_PAGE, 6);
}

/********************************************************************************************/
RC createNodePage(BTreeHandle *tree, PageNumber pageNum, int nodeState,
		int parent, int leftSibling, int rightSibling) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// get n
	int n;
	getN(tree, &n);

	// pin the page
	rc = pinPage(BM, page, pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	// init the content of the node page,
	// ip[0] - node state:
	//		0 is not inited
	//		1 is inited, non-leaf
	//		2 is inited, leaf
	//		3 is deleted, tombstone
	// ip[1] - current keys
	// ip[2] - parent
	// ip[3] - leftsibling
	// ip[4 + n + 2 * n] - rightsibling
	int *ip = (int *) page;
	ip[0] = nodeState;
	ip[1] = 0;
	ip[2] = parent;

	// if the node is a leaf node, set left and right sibling
	if (nodeState == 2) {
		ip[3] = leftSibling;

		// Also need to store right sibling
		// n keys and n + 1 key pointers
		ip[4 + n + 2 * n] = rightSibling;
	}

	// unpin the page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	// increment TotalNodePages by 1
	rc = -99;
	rc = setTotalNodePages(tree, ++pageNum);

	if (rc != RC_OK) {
		return rc;
	}

	// increment # of Nodes by 1
	int numNodes;
	rc = -99;
	rc = getNumNodes(tree, &numNodes);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = setNumNodes(tree, ++numNodes);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	return RC_OK;
}

RC getNodeState(BTreeHandle *tree, PageNumber pageNum, int *result) {
	return getPageInfo(tree, pageNum, 0, result);
}

RC setNodeState(BTreeHandle *tree, int value, PageNumber pageNum) {
	return setPageInfo(tree, value, pageNum, 0);
}

RC getCurrentKeys(BTreeHandle *tree, PageNumber pageNum, int *result) {
	return getPageInfo(tree, pageNum, 1, result);
}

RC setCurrentKeys(BTreeHandle *tree, int value, PageNumber pageNum) {
	return setPageInfo(tree, value, pageNum, 1);
}

RC getParent(BTreeHandle *tree, PageNumber pageNum, PageNumber *result) {
	return getPageInfo(tree, pageNum, 2, result);
}

RC setParent(BTreeHandle *tree, PageNumber value, PageNumber pageNum) {
	return setPageInfo(tree, (int) value, pageNum, 2);
}

RC getLeftSibling(BTreeHandle *tree, PageNumber pageNum, PageNumber *result) {
	return getPageInfo(tree, pageNum, 3, result);
}

RC setLeftSibling(BTreeHandle *tree, PageNumber value, PageNumber pageNum) {
	return setPageInfo(tree, (int) value, pageNum, 3);
}

RC getRightSibling(BTreeHandle *tree, PageNumber pageNum, PageNumber *result) {
	// get n
	int n;

	RC rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	return getPageInfo(tree, pageNum, 4 + n + 2 * n, result);
}

RC setRightSibling(BTreeHandle *tree, PageNumber value, PageNumber pageNum) {
	// get n
	int n;

	RC rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	return setPageInfo(tree, (int) value, pageNum, 4 + n + 2 * n);
}

RC getKey(BTreeHandle *tree, int keyNum, PageNumber pageNum, int *result) {
	// get n
	int n;

	RC rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	if (keyNum > n - 1) {
		return RC_KEY_NUM_TOO_LARGE;
	}

	// keyNum start from 0
	return getPageInfo(tree, pageNum, 4 + keyNum, result);
}

RC setKey(BTreeHandle *tree, int keyNum, int value, PageNumber pageNum) {
	// get n
	int n;

	RC rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	if (keyNum > n - 1) {
		return RC_KEY_NUM_TOO_LARGE;
	}

	// keyNum start from 0
	return setPageInfo(tree, value, pageNum, 4 + keyNum);
}

RC getKeyPointer(BTreeHandle *tree, int keyPointerNum, PageNumber pageNum,
		int *result) {
	// get n
	int n;

	RC rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	if (keyPointerNum > n) {
		return RC_KEY_NUM_TOO_LARGE;
	}

	// keyNum start from 0
	rc = -99;
	rc = getPageInfo(tree, pageNum, 4 + n + 2 * keyPointerNum, result);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = getPageInfo(tree, pageNum, 4 + n + 2 * keyPointerNum + 1, result + 1);

	if (rc != RC_OK) {
		return rc;
	}

	return RC_OK;
}

RC setKeyPointer(BTreeHandle *tree, int keyPointerNum, PageNumber keyPage,
		int keySlot, PageNumber pageNum) {
	// get n
	int n;

	RC rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	if (keyPointerNum > n) {
		return RC_KEY_NUM_TOO_LARGE;
	}

	// keyNum start from 0
	rc = -99;
	rc = setPageInfo(tree, keyPage, pageNum, 4 + n + 2 * keyPointerNum);

	if (rc != RC_OK) {
		return rc;
	}

	rc = -99;
	rc = setPageInfo(tree, keyPage, pageNum, 4 + n + 2 * keyPointerNum + 1);

	if (rc != RC_OK) {
		return rc;
	}

	return RC_OK;
}

/**************************************************************************************
 * Function Name: findKey
 *
 * Description:
 *		Get the RID the search key in given B+ Tree
 *		If not exist, return RC_IM_KEY_NOT_FOUND
 *
 * Parameters:
 * 		BTreeHandle *tree: Entry to the B+ Tree
 * 		Value *key: search key
 * 		RID *result: the RID for the key we found
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC findKey(BTreeHandle *tree, Value *key, RID *result) {
	RC rc;

	// get root page
	PageNumber rootPage;
	rc = -99;
	rc = getRootPage(tree, &rootPage);

	if (rc != RC_OK) {
		return rc;
	}

	BT_KeyPosition *kp;
	rc = -99;
	rc = searchKey(tree, rootPage ,key, result, kp);

	if (rc != RC_OK) {
		return rc;
	}

	return RC_OK;
}

/********************************************************************************************/
RC searchKey(BTreeHandle *tree, PageNumber nodePage, Value *key, RID *result,
		BT_KeyPosition *kp) {
	RC rc;

	// get node state
	int nodeState;
	rc = -99;
	rc = getNodeState(tree, nodePage, &nodeState);

	if (rc != RC_OK) {
		return rc;
	}

	// bottom up condition

	// try to find the key, if found, store the RID into result
	// if not, store the leaf node page number and key position into the result
	rc = -99;
	rc = searchMatchEntry(tree, nodePage, key, result, kp);

	if (rc != RC_OK) {
		return rc;
	}

	// if current node is a not a leaf, then search from its child again
	if (nodeState != 2) {
		rc = -99;
		rc = searchKey(tree, result->page, key, result, kp);

		if (rc != RC_OK) {
			return rc;
		}
	}

	return RC_OK;
}

/********************************************************************************************/
RC searchMatchEntry(BTreeHandle *tree, PageNumber nodePage, Value *key,
		RID *result, BT_KeyPosition *kp) {
	RC rc;

	int i = 0;
	int resultInt[2];

	// get n
	int n;
	rc = -99;
	rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	// get key
	int keyInt;
	rc = -99;
	rc = getKey(tree, i, nodePage, &keyInt);

	if (rc != RC_OK) {
		return rc;
	}

	while (i < n && key->v.intV > keyInt) {
		rc = -99;
		rc = getKey(tree, ++i, nodePage, &keyInt);

		if (rc != RC_OK) {
			return rc;
		}
	}

	// after the while loop, either i comes to the end of the key array (i == n)
	// or we found a key that meets (key->v.intV <= keyInt)

	// get the key pointer
	rc = -99;
	rc = getKeyPointer(tree, i, nodePage, resultInt);

	if (rc != RC_OK) {
		return rc;
	}

	// store the current node page number and key position
	kp->nodePage = nodePage; // leaf node page number
	kp->keyPos = i; // key position, starts from 0

	// store the key pointer
	result->page = resultInt[0];
	result->slot = resultInt[1];

	// get node state
	int nodeState;
	rc = -99;
	rc = getNodeState(tree, nodePage, &nodeState);

	if (rc != RC_OK) {
		return rc;
	}

	// if the current node is not node, return RC_KEY_SCOPE_FOUND
	if (nodeState != 2) {
		return RC_KEY_SCOPE_FOUND;
	}

	// if the current is a leaf node, then remove the situation of the key is found (key->v.intV == keyInt)
	// i comes to the end (i = n), or (i < n) if find a scope (key->v->intV < keyInt)
	// then return RC_IM_KEY_NOT_FOUND
	if (i == n || key->v.intV < keyInt) {
		return RC_IM_KEY_NOT_FOUND;
	}

	return RC_OK;
}

/********************************************************************************************/
RC insertKey(BTreeHandle *tree, Value *key, RID rid) {
	RC rc;

	// get root page
	PageNumber rootPage;
	rc = -99;
	rc = getRootPage(tree, &rootPage);

	if (rc != RC_OK) {
		return rc;
	}

	RID result;
	BT_KeyPosition *kp;
	rc = -99;
	rc = searchKey(tree, rootPage,key, &result, kp);

	// if return RC_OK, then the to-be inserted key is found, return RC_IM_KEY_ALREADY_EXISTS
	// else if return RC_IM_KEY_NOT_FOUND, then the to-be inserted key is not in the tree, insert it
	// else, some error happens, return rc
	if (rc == RC_OK) {
		return RC_IM_KEY_ALREADY_EXISTS;
	} else if (rc == RC_IM_KEY_NOT_FOUND) {
		// start to insert

		// get N
		int n;
		rc = -99;
		rc = getN(tree, &n);

		if (rc != RC_OK) {
			return rc;
		}

		// get current keys
		int currentKeys;

		rc = -99;
		rc = getCurrentKeys(tree, kp->nodePage, &currentKeys);

		if (rc != RC_OK) {
			return rc;
		}

		// if current node is full, split it into two nodes
		if (currentKeys == n) {
			// split
			splitNode();
		} else {
			rc = -99;
			rc = insertLeafNode(tree, kp, key, &rid);

			if (rc != RC_OK) {
				return rc;
			}
		}

		// increment # of entries by 1
		int numEntries;

		rc = -99;
		rc = getNumEntries(tree,  &numEntries);

		if (rc != RC_OK) {
			return rc;
		}

		rc = -99;
		rc = setNumEntries(tree, ++numEntries);

		if (rc != RC_OK) {
			return rc;
		}

	} else {
		return rc;
	}

	return RC_OK;
}

/********************************************************************************************/
RC insertLeafNode(BTreeHandle *tree, BT_KeyPosition *kp, Value *key, RID *rid) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// get N
	int n;
	rc = -99;
	rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	// pin the page
	rc = pinPage(BM, page, kp->nodePage);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;

	// get current keys
	int currentKeys = ip[1];

	// i is the index of key and key pointer, starts from 0
	int i;
	for (i = currentKeys - 1; i > kp->keyPos - 1; i--) {
		ip[4 + i + 1] = ip[4 + i];
		ip[4 + n + 2 * (i + 1)] = ip[4 + n + 2 * i];
		ip[4 + n + 2 * (i + 1) + 1] = ip[4 + n + 2 * i + 1];
	}

	// After the for loop, the i is pointing the left of the key position
	// put the key and RID into the right slot
	ip[4 + i + 1] = key->v.intV;
	ip[4 + n + 2 * (i + 1)] = rid->page;
	ip[4 + n + 2 * (i + 1) + 1] = rid->slot;

	// increment current
	ip[1]++;

	// unpin the page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	return RC_OK;
}

/********************************************************************************************/
RC splitNode(BTreeHandle *tree, BT_KeyPosition *kp, Value *key, RID *rid) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	BM_PageHandle *newPage = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;
	// currently the node has n keys and would be filled one more
	// split the node first - create a new node

	// get total node pages
	int totalNodePages;
	getTotalNodePages(tree, &totalNodePages);

	int newNode = totalNodePages;

	// get root page
	int rootPage;
	getRootPage(tree, &rootPage);

	// get n
	int n;
	getN(tree, &n);

	// pin the page
	rc = pinPage(BM, page, kp->nodePage);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;

	// get node state
	int nodeState = ip[0];

	// get current keys
	int currentKeys = ip[1];

	// get parent
	int parent = ip[2];

	// get right sibling
	int rightSibling = ip[4 + n + 2 * n];

	// THE NODE IS A LEAF
	if (nodeState == 2) {
		// copy the keys and key pointer into temp arrays
		int *keys = (int *) calloc(n + 1, sizeof(int)); // index is 0..n
		int *keyPointers = (int *) calloc(2 * (n + 2), sizeof(int)); // index is 0..2(n+1)+1

		memcpy((char *) keys, page->data + 4 * sizeof(int), n * sizeof(int));
		memcpy((char *) keyPointers, page->data + (4 + n) * sizeof(int),
				(2 * (n + 1)) * sizeof(int));

		// if i comes to the end of the array, then insert the new key and key pointer into the end of the array
		if (kp->keyPos == n) {
			keys[n] = key->v.intV;
			keyPointers[2 * (n + 1)] = rid->page;
			keyPointers[2 * (n + 1) + 1] = rid->slot;
		} else if (key->v.intV < keys[kp->keyPos]) {
			int j;
			for (j = n; j > kp->keyPos; j--) {
				keys[j] = keys[j - 1];
				keyPointers[2 * (j + 1)] = keyPointers[2 * j];
				keyPointers[2 * (j + 1) + 1] = keyPointers[2 * j + 1];
			}

			// after the for loop, the j comes to i (j = i)
			// insert the new key and key pointer into the j spot
			keys[j] = key->v.intV;
			keyPointers[2 * (j + 1)] = rid->page;
			keyPointers[2 * (j + 1) + 1] = rid->slot;
		}

		// get the new number of keys in the split node
		int newCurrentKeys = (int) ceil((double) n / 2.0f);

		// copy the first half into the original node
		memcpy(page->data + 4 * sizeof(int), (char *) keys,
				newCurrentKeys * sizeof(int));
		memcpy(page->data + (4 + n) * sizeof(int), (char *) keyPointers,
				(2 * newCurrentKeys) * sizeof(int));

		// unset the rest keys and key pointers
		memset(page->data + (4 + newCurrentKeys) * sizeof(int), 0,
				(n - newCurrentKeys) * sizeof(int));
		memset(page->data + (4 + n + 2 * (newCurrentKeys)) * sizeof(int), 0,
				2 * (n + 1 - newCurrentKeys) * sizeof(int));

		// set right sibling to the new node page
		ip[4 + n + 2 * n] = newNode;

		// create the new node, the new node page number is totalNodePages
		createNodePage(tree, newNode, nodeState, parent, kp->nodePage,
				rightSibling);

		// init the new node page
		// pin the page
		rc = pinPage(BM, newPage, newNode);

		if (rc != RC_OK) {
			return rc;
		}

		int *nip = (int *) newPage->data;

		nip[1] = newCurrentKeys;

		memcpy(newPage->data + 4 * sizeof(int), (char *) keys,
				newCurrentKeys * sizeof(int));
		memcpy(newPage->data + (4 + n) * sizeof(int), (char *) keyPointers,
				(2 * newCurrentKeys) * sizeof(int));

		// unpin the page
		rc = -99;
		rc = unpinPage(BM, newPage);

		if (rc != RC_OK) {
			return rc;
		}

		int pCurrentKeys;
		getCurrentKeys(tree, parent, &pCurrentKeys);

		// insert the smallest key in the right node into the parent
		if (pCurrentKeys < n) {
			// case 1 - parent is not full
			insertInternalNode();
		} else if (pCurrentKeys == n) {
			BT_KeyPosition *newKp;
			newKp->nodePage = parent;
			newKp->keyPos = -1;

			RID *newRid;
			newRid->page = newNode;
			newRid->slot = -1;

			Value *newKey;
			newKey->dt = DT_INT;
			newKey->v.intV = keys[newCurrentKeys];
			//newKey->v->intV = keys[newCurrentKeys];

			splitNode(tree, newKp, newKey, newRid);
		}
	} else if (kp->nodePage != rootPage) {
		// case 2 - parent is full, parent is not root

	} else if (kp->nodePage == rootPage) {
		// case 3 - parent is full, parent is root
	}

	// unpin the page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	return RC_OK;
}

RC insertInternalNode(BTreeHandle *tree ) {
	// insert the new key and key
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	BT_KeyPosition *newKp =(BT_KeyPosition *) malloc(sizeof(BT_KeyPosition));

	// get n
	int n;

	int newCurrentKeys = (int) ceil((double) n / 2.0f);

	getN(tree, &n);
	int *keys = (int *) calloc(n + 1, sizeof(int));

	int *ip = (int *) page->data;

	int parent = ip[2];

	// get total node pages
	int totalNodePages;
	getTotalNodePages(tree, &totalNodePages);

	int newNode = totalNodePages;

	newKp->nodePage = parent;
	newKp->keyPos = -1;

	RID *newRid;
	newRid->page = newNode;
	newRid->slot = -1;

	Value *newKey;
	newKey->dt = DT_INT;
	newKey->v.intV = keys[newCurrentKeys];
	//newKey->v->intV = keys[newCurrentKeys];

	splitNode(tree, newKp, newKey, newRid);

	return RC_OK;
}

/********************************************************************************************/
RC deleteKey(BTreeHandle *tree, Value *key) {
	RC rc;

	// get root page
	PageNumber rootPage;
	rc = -99;
	rc = getRootPage(tree, &rootPage);

	if (rc != RC_OK) {
		return rc;
	}

	RID result;
	BT_KeyPosition *kp;
	rc = -99;
	rc = searchKey(tree,rootPage ,key, &result, kp);

	if (rc == RC_KEY_SCOPE_FOUND) {
		return RC_IM_KEY_NOT_FOUND;
	} else if (rc != RC_OK) {
		return rc;
	}

	// the key is found, delete it

	// get N
	int n;
	rc = -99;
	rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	// get current keys
	int currentKeys;

	rc = -99;
	rc = getCurrentKeys(tree, kp->nodePage, &currentKeys);

	if (rc != RC_OK) {
		return rc;
	}

	// if current node is full, split it into two nodes
	if (currentKeys >= (int) ceil((double) n / 2.0f)) {
		rc = -99;
		rc = deleteOldKey(tree, kp);

		if (rc != RC_OK) {
			return rc;
		}
	} else {
		// combine
		combineNode(tree,kp);
	}
	return RC_OK;
}

/********************************************************************************************/
RC combineNode(BTreeHandle *tree, BT_KeyPosition *kp) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	rc = pinPage(BM, page, kp->nodePage);

	if (rc != RC_OK) {
		return rc;
	}



	return RC_OK;
}

/********************************************************************************************/
RC deleteOldKey(BTreeHandle *tree, BT_KeyPosition *kp) {
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	RC rc;

	// get N
	int n;
	rc = -99;
	rc = getN(tree, &n);

	if (rc != RC_OK) {
		return rc;
	}

	// pin the page
	rc = pinPage(BM, page, kp->nodePage);

	if (rc != RC_OK) {
		return rc;
	}

	int *ip = (int *) page->data;

	// get current keys
	int currentKeys = ip[1];

	int i;
	for (i = kp->keyPos + 1; i < currentKeys; i++) {
		ip[4 + i] = ip[4 + i + 1];
		ip[4 + n + 2 * i] = ip[4 + n + 2 * (i + 1)];
		ip[4 + n + 2 * i + 1] = ip[4 + n + 2 * (i + 1) + 1];
	}

	// After the for loop, the i is pointing the currentKeys position
	// unset the key and keyPointer of currentKeys position
	ip[4 + i] = 0;
	ip[4 + n + 2 * i] = 0;
	ip[4 + n + 2 * i + 1] = 0;

	// decrement current
	ip[1]--;

	// unpin the page
	rc = -99;
	rc = unpinPage(BM, page);

	if (rc != RC_OK) {
		return rc;
	}

	free(page);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: openTreeScan
 *
 * Description:
 *		Create a new scan handler with given B+ Tree
 *
 * Parameters:
 * 		BTreeHandle *tree: Entry to the B+ Tree
 * 		BT_ScanHandle **handle: pointer to an array of scan handlers, only use the first spot
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle) {
	BT_KeyPosition *kp = (BT_KeyPosition *) malloc(sizeof(BT_KeyPosition));

	// set kp
	// kp->nodePage = 0;
	// kp->keyPos = -1;

	(*handle)->tree = tree;
	(*handle)->mgmtData = kp;

	return RC_OK;
}
/**************************************************************************************
 * Function Name: nextEntry
 *
 * Description:
 *		Get the next entry RID after the given RID in the given B+ Tree
 *		If RID is null, start from beginning
 *
 * Parameters:
 * 		BT_ScanHandle *handle: Entry to the scan handler
 * 		RID *result: The starting RID, after running it's written with the found RID
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC nextEntry(BT_ScanHandle *handle, RID *result) {

	return RC_OK;
}

/**************************************************************************************
 * Function Name: closeTreeScan
 *
 * Description:
 *		Close given scan handler
 *
 * Parameters:
 * 		BT_ScanHandle *handle: Entry to the scan handler
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
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
RC closeTreeScan(BT_ScanHandle *handle) {
	free(handle->mgmtData);
	handle->tree = NULL;
	handle->mgmtData = NULL;

	return RC_OK;
}

////////////////////////////// debug and test functions ///////////////////////////////////
/**************************************************************************************
 * Function Name: printTree
 *
 * Description:
 *		Parse a B+ Tree into string in the following format:
 *			(0)[1,13,2,23,3]
 *			(1)[1.1,1,2.3,11,2]
 *			(2)[1.2,13,3.5,17,3]
 *			(3)[4.4,23,3.2,52]
 *
 * Parameters:
 * 		BTreeHandle *tree: Entry to the B+ Tree
 *
 * Return:
 *		char *: String version of a B+ Tree
 *
 * Author:
 *		Xiaolang Wang <xwang122@hawk.iit.edu>
 *
 * History:
 *		Date        Name                                      Content
 *		----------  ----------------------------------------  ------------------------
 *		2015-04-27  Xiaolang Wang <xwang122@hawk.iit.edu>     Initialization.
 **************************************************************************************/
/*char *printTree(BTreeHandle *tree) {

}*/

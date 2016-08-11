#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "dt.h"
#include "expr.h"
#include "record_mgr.h"
#include "storage_mgr.h"
#include "tables.h"
#include "test_helper.h"

// struct for test records
typedef struct TestRecord {
	int a;
	char *b;
	int c;
} TestRecord;

// helper methods
Record *testRecord(Schema *schema, int a, char *b, int c);
Schema *testSchema(void);
Record *formTestRecord(Schema *schema, TestRecord in);

char *testName;

int main() {
	testName = "test of Conditional Update";
	/////////////////////////// test of checkEqualsExpr ////////////////////////////////
	printf("/////////// test of checkEqualsExpr /////////////////\n");
	Expr *lIn, *rIn, *testCond;
	Value *rVal;
	RC rc;

	//make a OP_COMP_SMALLER expression
	//should return RC_NOT_EQUALS_EXPR (160)
	MAKE_ATTRREF(lIn, 2);
	MAKE_VALUE(rVal, DT_INT, 3);
	MAKE_CONS(rIn, rVal);
	MAKE_BINOP_EXPR(testCond, lIn, rIn, OP_COMP_SMALLER);
	rc = checkEqualsExpr(testCond);

	printf("rc = %d\n", rc);
	freeExpr(testCond);

	//make a OP_COMP_EQUAL expression with right side is not a constant
	//should return RC_RIGHT_NOT_CONS (162)
	MAKE_ATTRREF(lIn, 2);
	MAKE_ATTRREF(rIn, 2);
	MAKE_BINOP_EXPR(testCond, lIn, rIn, OP_COMP_EQUAL);
	rc = checkEqualsExpr(testCond);

	printf("rc = %d\n", rc);
	freeExpr(testCond);

	//make a valid OP_COMP_EQUAL expression
	//should return RC_OK (0)
	MAKE_ATTRREF(lIn, 2);
	MAKE_VALUE(rVal, DT_INT, 3);
	MAKE_CONS(rIn, rVal);
	MAKE_BINOP_EXPR(testCond, lIn, rIn, OP_COMP_EQUAL);
	rc = checkEqualsExpr(testCond);

	printf("rc = %d\n", rc);
	freeExpr(testCond);

	/////////////////////////// test of condUpdateRecord ////////////////////////////////
	printf("/////////// test of condUpdateRecord /////////////////\n");

	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = { { 1, "aaaa", 3 }, { 2, "bbbb", 2 },
			{ 3, "cccc", 1 }, { 4, "dddd", 3 }, { 5, "eeee", 5 },
			{ 6, "ffff", 1 }, { 7, "gggg", 3 }, { 8, "hhhh", 3 },
			{ 9, "iiii", 2 }, { 10, "jjjj", 5 }, { 11, "kkkk", 2 }, { 12,
					"llll", 5 } };

	int numInserts = 12;
	int i, j;
	Record *r;
	RID *rids;
	Schema *schema;
	RM_ScanHandle *sc = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));

	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	initRecordManager(NULL);
	createTable("test_table_r", schema);
	openTable(table, "test_table_r");

	// insert rows into table
	for (i = 0; i < numInserts; i++) {
		r = formTestRecord(schema, inserts[i]);
		insertRecord(table, r);
		rids[i] = r->id;
	}

	//create scaner
	Expr *left, *right, *scanCond;
	Value *rV;
	MAKE_ATTRREF(left, 2);
	MAKE_VALUE(rV, DT_INT, 3);
	MAKE_CONS(right, rV);
	MAKE_BINOP_EXPR(scanCond, left, right, OP_COMP_SMALLER);

	startScan(table, sc, scanCond);

	//create test condition
	MAKE_ATTRREF(lIn, 1);
	MAKE_CONS(rIn, stringToValue("stest"));
	MAKE_BINOP_EXPR(testCond, lIn, rIn, OP_COMP_EQUAL);

	//run condUpdateRecord
	condUpdateRecord(sc, testCond);

	//print out records now
	//the 2nd, 3rd, 6th, 9th, 11th records should says "test"
	for (i = 0; i < numInserts; i++) {
		getRecord(table, rids[i], r);
		for (j = 0; j < 4; j++) {
			printf("%c\n", *(r->data + 4 + j));
		}
	}

	closeTable(table);
	deleteTable("test_table_r");
	shutdownRecordManager();
	free(table);
	free(sc);
	freeExpr(scanCond);
	freeExpr(testCond);

	TEST_DONE()
	;
	return 0;
}

Schema *
testSchema(void) {
	Schema *result;
	char *names[] = { "a", "b", "c" };
	DataType dt[] = { DT_INT, DT_STRING, DT_INT };
	int sizes[] = { 0, 4, 0 };
	int keys[] = { 0 };
	int i;
	char **cpNames = (char **) malloc(sizeof(char*) * 3);
	DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
	int *cpSizes = (int *) malloc(sizeof(int) * 3);
	int *cpKeys = (int *) malloc(sizeof(int));

	for (i = 0; i < 3; i++) {
		cpNames[i] = (char *) malloc(2);
		strcpy(cpNames[i], names[i]);
	}
	memcpy(cpDt, dt, sizeof(DataType) * 3);
	memcpy(cpSizes, sizes, sizeof(int) * 3);
	memcpy(cpKeys, keys, sizeof(int));

	result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

	return result;
}
Record *
formTestRecord(Schema *schema, TestRecord in) {
	return testRecord(schema, in.a, in.b, in.c);
}

Record *
testRecord(Schema *schema, int a, char *b, int c) {
	Record *result;
	Value *value;

	createRecord(&result, schema);

	MAKE_VALUE(value, DT_INT, a);
	setAttr(result, schema, 0, value);
	freeVal(value);

	MAKE_STRING_VALUE(value, b);
	setAttr(result, schema, 1, value);
	freeVal(value);

	MAKE_VALUE(value, DT_INT, c);
	setAttr(result, schema, 2, value);
	freeVal(value);

	return result;
}

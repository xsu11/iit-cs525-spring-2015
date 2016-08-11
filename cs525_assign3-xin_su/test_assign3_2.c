#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"

#define ASSERT_EQUALS_RECORDS(_l,_r, schema, message)			\
  do {									\
    Record *_lR = _l;                                                   \
    Record *_rR = _r;                                                   \
    ASSERT_TRUE(memcmp(_lR->data,_rR->data,getRecordSize(schema)) == 0, message); \
    int i;								\
    for(i = 0; i < schema->numAttr; i++)				\
      {									\
        Value *lVal, *rVal;                                             \
		char *lSer, *rSer; \
        getAttr(_lR, schema, i, &lVal);                                  \
        getAttr(_rR, schema, i, &rVal);                                  \
		lSer = serializeValue(lVal); \
		rSer = serializeValue(rVal); \
        ASSERT_EQUALS_STRING(lSer, rSer, "attr same");	\
		free(lVal); \
		free(rVal); \
		free(lSer); \
		free(rSer); \
      }									\
  } while(0)

#define ASSERT_EQUALS_RECORD_IN(_l,_r, rSize, schema, message)		\
  do {									\
    int i;								\
    boolean found = false;						\
    for(i = 0; i < rSize; i++)						\
      if (memcmp(_l->data,_r[i]->data,getRecordSize(schema)) == 0)	\
	found = true;							\
    ASSERT_TRUE(0, message);						\
  } while(0)

#define OP_TRUE(left, right, op, message)		\
  do {							\
    Value *result = (Value *) malloc(sizeof(Value));	\
    op(left, right, result);				\
    bool b = result->v.boolV;				\
    free(result);					\
    ASSERT_TRUE(b,message);				\
   } while (0)

// test methods
static void testRecords(void);
static void testCreateTableAndInsert(void);
static void testUpdateTable(void);
static void testScans(void);
static void testScansTwo(void);
static void testInsertManyRecords(void);
static void testMultipleScans(void);

// struct for test records
typedef struct TestRecord {
	int a;
	char *b;
	int c;
} TestRecord;

// helper methods
Record *testRecord(Schema *schema, int a, char *b, int c);
Schema *testSchema(void);
Record *fromTestRecord(Schema *schema, TestRecord in);

// test name
char *testName;

// main method
int main(void) {
	testName = "";
	testCreateTableAndInsert();
	return 0;
}

// ************************************************************ 
void testCreateTableAndInsert(void) {
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = { { 1, "aaaa", 3 }, { 2, "bbbb", 2 },
			{ 3, "cccc", 1 }, { 4, "dddd", 3 }, { 5, "eeee", 5 },
			{ 6, "ffff", 1 }, { 7, "gggg", 3 }, { 8, "hhhh", 3 },
			{ 9, "iiii", 2 } };
	int numInserts = 35000, i;
	Record *r;
	RID *rids;
	Schema *schema;
	testName = "test creating a new table and inserting tuples";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_r", schema));
	TEST_CHECK(openTable(table, "test_table_r"));

	// insert rows into table
	for (i = 0; i < numInserts; i++) {
		r = fromTestRecord(schema, inserts[i % 9]);
		TEST_CHECK(insertRecord(table, r));
		rids[i] = r->id;
		freeRecord(r);
		r = NULL;
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(openTable(table, "test_table_r"));

	createRecord(&r, schema);
	char *cp = (char *) calloc(5, sizeof(char));
	// randomly retrieve records from the table and compare to inserted ones
	for (i = 0; i < 35000; i++) {

		RID rid = rids[i];
		TEST_CHECK(getRecord(table, rid, r));

		int *ip = (int *) r->data;

		ASSERT_EQUALS_INT(inserts[i % 9].a, ip[0], "compare records");

		memcpy(cp, r->data + sizeof(int), 4);
		cp[4] = '\0';
		ASSERT_EQUALS_STRING(inserts[i % 9].b, cp, "compare records");

		ip = (int *) (r->data + sizeof(int) + sizeof(char) * 3000);

		ASSERT_EQUALS_INT(inserts[i % 9].c, ip[0], "compare records");
	}

	int totalPages = getTotalPages(table);
	ASSERT_EQUALS_INT(35003, totalPages, "totalPages");

	int p = searchFirstFreePage(table);
	ASSERT_EQUALS_INT(35003, p, "firstFreePage");

	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	free(cp);
	freeRecord(r);
	free(rids);
	free(table);
	TEST_DONE()
	;
}

Schema *
testSchema(void) {
	Schema *result;
	char *names[] = { "a", "b", "c" };
	DataType dt[] = { DT_INT, DT_STRING, DT_INT };
	int sizes[] = { 0, 3000, 0 };
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
fromTestRecord(Schema *schema, TestRecord in) {
	return testRecord(schema, in.a, in.b, in.c);
}

Record *
testRecord(Schema *schema, int a, char *b, int c) {
	Record *result;
	Value *value;

	TEST_CHECK(createRecord(&result, schema));

	MAKE_VALUE(value, DT_INT, a);
	TEST_CHECK(setAttr(result, schema, 0, value));
	freeVal(value);

	MAKE_STRING_VALUE(value, b);
	TEST_CHECK(setAttr(result, schema, 1, value));
	freeVal(value);

	MAKE_VALUE(value, DT_INT, c);
	TEST_CHECK(setAttr(result, schema, 2, value));
	freeVal(value);

	return result;
}

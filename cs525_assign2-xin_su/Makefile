CC=gcc
CFLAGS=-lpthread
DEPS=dberror.h storage_mgr.h buffer_mgr_stat.h buffer_mgr.h dt.h test_helper.h

OBJ1=dberror.o storage_mgr.o
OBJ2=dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

all: test_assign1_1 test_assign1_2 test_assign2_1_modified test_assign2_2

# assign 1
# test_assign1_1
test_assign1_1:	$(OBJ1)	test_assign1_1.o
	$(CC) -o $@ $^

dberror.o: dberror.c
	$(CC) -c dberror.c

storage_mgr.o: storage_mgr.c
	$(CC) -c storage_mgr.c

test_assign1_1.o: test_assign1_1.c
	$(CC) -c test_assign1_1.c

# test_assign1_2
test_assign1_2:	$(OBJ1)  test_assign1_2.o
	$(CC) -o $@ $^

test_assign1_2.o: test_assign1_2.c
	$(CC) -c test_assign1_2.c


# assign 2
# test_assign2_1_modified
test_assign2_1_modified: $(OBJ2)	test_assign2_1_modified.o
	$(CC) -o $@ $^ $(CFLAGS)

buffer_mgr_stat.o: buffer_mgr_stat.c
	$(CC) -c buffer_mgr_stat.c

buffer_mgr.o: buffer_mgr.c
	$(CC) -c buffer_mgr.c

test_assign2_1_modified.o: test_assign2_1_modified.c
	$(CC) -c test_assign2_1_modified.c

# test_assign2_2
test_assign2_2: $(OBJ2)	test_assign2_2.o
	$(CC) -o $@ $^ $(CFLAGS)

test_assign2_2.o: test_assign2_2.c
	$(CC) -c test_assign2_2.c

.PHONY: clean
clean:
	-rm -f test_assign1_1 test_assign1_2 test_assign2_1_modified test_assign2_2 *.o *.log


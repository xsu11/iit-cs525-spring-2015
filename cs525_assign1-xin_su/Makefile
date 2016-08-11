CC=gcc
## CFLAGS=-I
DEPS=dberror.h storage_mgr.h test_helper.h

OBJ=dberror.o storage_mgr.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

all: test_assign1_1 test_assign1_2

test_assign1_1:	$(OBJ) test_assign1_1.o
	$(CC) -o $@ $^

dberror.o: dberror.c
	$(CC) -c dberror.c

storage_mgr.o: storage_mgr.c
	$(CC) -c storage_mgr.c

test_assign1_1.o: test_assign1_1.c
	$(CC) -c test_assign1_1.c


test_assign1_2:	$(OBJ)  test_assign1_2.o
	$(CC) -o $@ $^

test_assign1_2.o: test_assign1_2.c
	$(CC) -c test_assign1_2.c

.PHONY: clean
clean:
	-rm -f test_assign1_1 test_assign1_2 *.o *.log


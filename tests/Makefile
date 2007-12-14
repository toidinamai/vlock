include ../config.mk

VPATH = ../src

override CFLAGS+=-I../src

export VLOCK_TEST_OUTPUT_MODE
VLOCK_TEST_OUTPUT_MODE = verbose

.PHONY: all
all: run-tests

vlock-test : LDFLAGS += -lcunit
vlock-test: \
	list.o \
	test_list.o \
	tsort.o \
	test_tsort.o \
	vlock-test.o

test_list.o: test_list.c test_list.h list.h
test_sort.o: test_tsort.c test_tsort.h tsort.h

.PHONY: run-tests
run-tests: vlock-test
	@./vlock-test

.PHONY: run-valgrind
run-valgrind : VLOCK_TEST_OUTPUT_MODE=silent
run-valgrind: vlock-test
	@valgrind \
		--tool=memcheck \
		--suppressions=.valgrind-supressions \
		--error-exitcode=1 \
		--leak-check=full \
		--show-reachable=yes \
		./vlock-test

.PHONY: clean
clean:
	$(RM) vlock-test $(wildcard *.o)

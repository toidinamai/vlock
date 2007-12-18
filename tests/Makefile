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
	util.o \
	test_util.o \
	process.o \
	test_process.o \
	vlock-test.o

test_list.o: test_list.c test_list.h list.h
test_tsort.o: test_tsort.c test_tsort.h tsort.h
test_util.o: test_util.c test_util.h util.h
test_process.o: test_process.c test_process.h process.h

.PHONY: check
check:
	@./vlock-test

.PHONY: memcheck
memcheck : VLOCK_TEST_OUTPUT_MODE=silent
memcheck: vlock-test
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

include ../config.mk

override CFLAGS+=-I../src

.PHONY: all
all: run-tests

vlock-test : LDFLAGS += -lcunit
vlock-test: vlock-test.o test_list.o

test_list.o: test_list.c test_list.h

.PHONY: run-tests
run-tests: vlock-test
	./vlock-test

.PHONY: clean
clean:
	$(RM) vlock-test $(wildcard *.o)

include ../config.mk

override CFLAGS+=-I../src

.PHONY: all
all: run-tests

vlock-test : LDFLAGS += -lcunit
vlock-test: vlock-test.o

.PHONY: run-tests
run-tests: vlock-test
	./vlock-test

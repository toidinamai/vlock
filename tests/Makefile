include ../config.mk

override CFLAGS+=-I../src

.PHONY: all
all: run-tests

.PHONY: run-tests
run-tests: vlock-test
	./vlock-test

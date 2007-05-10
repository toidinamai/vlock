CC = gcc
CFLAGS = -O2

OBJS = vlock.o signals.o help.o

vlock: $(OBJS)
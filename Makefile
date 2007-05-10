CC = gcc
CFLAGS = -O2

OBJS = vlock.o signals.o help.o terminal.o input.o

vlock: $(OBJS)

vlock.o: vlock.h version.h
signals.o: vlock.h
help.o: vlock.h
terminal.o: vlock.h
input.o: vlock.h

clean:
	rm -f $(OBJS) vlock core.vlock

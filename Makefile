# vlock makefile

CC = gcc
# remove the -DUSE_PAM, -ldl, and -lpam if you aren't using PAM
CFLAGS = -O2 -DUSE_PAM
LDFLAGS = -ldl -lpam

OBJS = vlock.o signals.o help.o terminal.o input.o

vlock: $(OBJS)

vlock.man: vlock.1
	groff -man -Tascii vlock.1 > vlock.man

vlock.o: vlock.h version.h
signals.o: vlock.h
help.o: vlock.h
terminal.o: vlock.h
input.o: vlock.h

clean:
	rm -f $(OBJS) vlock core core.vlock


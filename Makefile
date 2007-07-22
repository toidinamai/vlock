# vlock makefile

include config.mk

OBJS = vlock.o signals.o help.o terminal.o input.o sysrq.o
PROGRAMS = vlock vlock-auth vlock-grab vlock-lockswitch vlock-unlockswitch vlock-nosysrq

.PHONY: all
all: $(PROGRAMS)

vlock: $(OBJS)

vlock.man: vlock.1
	groff -man -Tascii $< > $@

vlock.1.html: vlock.1
	groff -man -Thtml $< > $@

vlock.o: vlock.h version.h
signals.o: vlock.h
help.o: vlock.h
terminal.o: vlock.h
input.o: vlock.h

.PHONY: install
install: vlock
	$(INSTALL) -D -m 4711 -o root -g root vlock $(DESTDIR)$(PREFIX)/bin/vlock
	$(INSTALL) -D -m 644 -o root -g root vlock.1 $(DESTDIR)$(PREFIX)/share/man/man1/vlock.1

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS) vlock.man vlock.1.html

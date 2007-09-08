# vlock makefile

include config.mk

override CFLAGS += -Isrc -DPREFIX="\"$(PREFIX)\""

VPATH = src

VLOCK_VERSION = 2.1 rc1

PROGRAMS = \
					vlock \
					vlock-current \
					vlock-new \
					vlock-all \
					vlock-nosysrq

.PHONY: all
all: $(PROGRAMS)

vlock: vlock.sh config.mk Makefile
	$(BOURNE_SHELL) -n $<
	sed \
		-e 's,%BOURNE_SHELL%,$(BOURNE_SHELL),' \
		-e 's,%PREFIX%,$(PREFIX),' \
		-e 's,%VLOCK_VERSION%,$(VLOCK_VERSION),' \
		$< > $@.tmp
	mv -f $@.tmp $@

ifneq ($(USE_ROOT_PASS),y)
vlock-current : override CFLAGS += -DNO_ROOT_PASS
endif

ifeq ($(AUTH_METHOD),pam)
vlock-current : override LDFLAGS += $(PAM_LIBS)
endif

ifeq ($(AUTH_METHOD),shadow)
vlock-current : override LDFLAGS += -lcrypt
endif

vlock-current: vlock-current.c prompt.c auth-$(AUTH_METHOD).c

ifeq ($(USE_PAM),y)
vlock-nosysrq vlock-all : override LDFLAGS += $(PAM_LIBS)
vlock-nosysrq vlock-all : override CFLAGS += -DUSE_PAM
endif

ifndef VLOCK_GROUP
VLOCK_GROUP = root
ifndef VLOCK_MODE
VLOCK_MODE = 4711
endif
else # VLOCK_GROUP is defined
ifndef VLOCK_MODE
VLOCK_MODE = 4710
endif
endif

.PHONY: install
install: install-programs install-man

.PHONY: install-programs
install-programs: $(PROGRAMS)
	$(INSTALL) -D -m 755 -o root -g $(ROOT_GROUP) vlock $(DESTDIR)$(PREFIX)/bin/vlock
	$(INSTALL) -D -m 4711 -o root -g $(ROOT_GROUP) vlock-current $(DESTDIR)$(PREFIX)/sbin/vlock-current
	$(INSTALL) -D -m 755 -o root -g $(ROOT_GROUP) vlock-all $(DESTDIR)$(PREFIX)/sbin/vlock-all
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-nosysrq $(DESTDIR)$(PREFIX)/sbin/vlock-nosysrq
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-new $(DESTDIR)$(PREFIX)/sbin/vlock-new

.PHONY: install-man
install-man:
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock.1 $(DESTDIR)$(PREFIX)/share/man/man1/vlock.1
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-current.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-current.8
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-all.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-all.8
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-new.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-new.8
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-nosysrq.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-nosysrq.8

.PHONY: clean
clean:
	rm -f $(PROGRAMS)

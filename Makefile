# vlock makefile

include config.mk

VLOCK_VERSION = "2.0 beta1"

PROGRAMS = \
					 vlock \
					 vlock-lock \
					 vlock-new \
					 vlock-grab \
					 vlock-lockswitch \
					 vlock-unlockswitch \
					 vlock-nosysrq

.PHONY: all
all: $(PROGRAMS)

.PHONY: vlock
vlock: vlock.sh config.mk Makefile
	$(BOURNE_SHELL) -n $<
	sed \
		-e 's,%BOURNE_SHELL%,$(BOURNE_SHELL),' \
		-e 's,%PREFIX%,$(PREFIX),' \
		-e 's,%VLOCK_VERSION%,$(VLOCK_VERSION),' \
		$< > $@.tmp
	mv -f $@.tmp $@

ifneq ($(USE_ROOT_PASS),y)
vlock-lock : override CFLAGS += -DNO_ROOT_PASS
endif

ifeq ($(AUTH_METHOD),pam)
vlock-lock : override LDFLAGS += $(PAM_LIBS)
endif

ifeq ($(AUTH_METHOD),shadow)
vlock-lock : override LDFLAGS += -lcrypt
endif

vlock-lock: vlock-lock.c auth-$(AUTH_METHOD).c

ifeq ($(USE_PAM),y)
vlock-nosysrq vlock-grab : override LDFLAGS += $(PAM_LIBS)
vlock-nosysrq vlock-grab : override CFLAGS += -DUSE_PAM
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
	$(INSTALL) -D -m 755 -o root -g root vlock $(DESTDIR)$(PREFIX)/bin/vlock
	$(INSTALL) -D -m 4711 -o root -g root vlock-lock $(DESTDIR)$(PREFIX)/sbin/vlock-lock
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-grab $(DESTDIR)$(PREFIX)/sbin/vlock-grab
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-nosysrq $(DESTDIR)$(PREFIX)/sbin/vlock-nosysrq
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-new $(DESTDIR)$(PREFIX)/sbin/vlock-new
	$(INSTALL) -D -m 755 -o root -g root vlock-lockswitch $(DESTDIR)$(PREFIX)/sbin/vlock-lockswitch
	$(INSTALL) -D -m 755 -o root -g root vlock-unlockswitch $(DESTDIR)$(PREFIX)/sbin/vlock-unlockswitch

.PHONY: install-man
install-man:
	$(INSTALL) -D -m 644 -o root -g root man/vlock.1 $(DESTDIR)$(PREFIX)/share/man/man1/vlock.1
	$(INSTALL) -D -m 644 -o root -g root man/vlock-lock.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-lock.8
	$(INSTALL) -D -m 644 -o root -g root man/vlock-grab.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-grab.8
	$(INSTALL) -D -m 644 -o root -g root man/vlock-new.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-new.8
	$(INSTALL) -D -m 644 -o root -g root man/vlock-nosysrq.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-nosysrq.8
	$(INSTALL) -D -m 644 -o root -g root man/vlock-lockswitch.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-lockswitch.8
	$(INSTALL) -D -m 644 -o root -g root man/vlock-unlockswitch.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-unlockswitch.8

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS) vlock.man vlock.1.html

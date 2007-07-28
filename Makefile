# vlock makefile

include config.mk

VLOCK_VERSION = "vlock version 1.4"

PROGRAMS = vlock vlock-lock vlock-grab vlock-lockswitch vlock-unlockswitch vlock-nosysrq

.PHONY: all
all: $(PROGRAMS)

.PHONY: vlock
vlock: vlock.sh
	$(BOURNE_SHELL) -n $<
	sed \
		-e 's,%BOURNE_SHELL%,$(BOURNE_SHELL),' \
		-e 's,%PREFIX%,$(PREFIX),' \
		-e 's,%VLOCK_VERSION%,$(VLOCK_VERSION),' \
		$< > $@

ifneq ($(USE_ROOT_PASS),y)
vlock-lock : CFLAGS += -DNO_ROOT_PASS
endif

ifeq ($(AUTH_METHOD),pam)
vlock-lock : LDFLAGS += $(PAM_LIBS)
endif

ifeq ($(AUTH_METHOD),shadow)
vlock-lock : LDFLAGS += -lcrypt
endif

vlock-lock: vlock-lock.c auth-$(AUTH_METHOD).c

ifeq ($(USE_PAM_PERM),y)
vlock-nosysrq vlock-grab : LDFLAGS += $(PAM_LIBS)
endif

vlock.man: vlock.1
	groff -man -Tascii $< > $@

vlock.1.html: vlock.1
	groff -man -Thtml $< > $@

.PHONY: install
install: $(PROGRAMS)
	$(INSTALL) -D -m 755 -o root -g root vlock $(DESTDIR)$(PREFIX)/bin/vlock
	$(INSTALL) -D -m 4711 -o root -g root vlock-lock $(DESTDIR)$(PREFIX)/sbin/vlock-lock
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-grab $(DESTDIR)$(PREFIX)/sbin/vlock-grab
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-nosysrq $(DESTDIR)$(PREFIX)/sbin/vlock-nosysrq
	$(INSTALL) -D -m 755 -o root -g root vlock-lockswitch $(DESTDIR)$(PREFIX)/sbin/vlock-lockswitch
	$(INSTALL) -D -m 755 -o root -g root vlock-unlockswitch $(DESTDIR)$(PREFIX)/sbin/vlock-unlockswitch
	$(INSTALL) -D -m 644 -o root -g root vlock.1 $(DESTDIR)$(PREFIX)/share/man/man1/vlock.1

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS) vlock.man vlock.1.html

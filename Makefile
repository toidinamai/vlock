# vlock makefile

include config.mk

VPATH = src

VLOCK_VERSION = "2.1 alpha1"

PROGRAMS = \
					vlock \
					vlock-main \
					vlock-new \
					vlock-all \
					vlock-nosysrq

.PHONY: all
all: $(PROGRAMS) plugins

.PHONY: plugins
plugins:
	@$(MAKE) -C plugins

### installation rules ###

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
install: install-programs install-plugins install-man

.PHONY: install-programs
install-programs: $(PROGRAMS)
	$(INSTALL) -D -m 755 -o root -g $(ROOT_GROUP) vlock $(DESTDIR)$(PREFIX)/bin/vlock
	$(INSTALL) -D -m 4711 -o root -g $(ROOT_GROUP) vlock-main $(DESTDIR)$(PREFIX)/sbin/vlock-main
	$(INSTALL) -D -m 755 -o root -g $(ROOT_GROUP) vlock-all $(DESTDIR)$(PREFIX)/sbin/vlock-all
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-nosysrq $(DESTDIR)$(PREFIX)/sbin/vlock-nosysrq
	$(INSTALL) -D -m $(VLOCK_MODE) -o root -g $(VLOCK_GROUP) vlock-new $(DESTDIR)$(PREFIX)/sbin/vlock-new

.PHONY: install-plugins
install-plugins:
	@$(MAKE) -C plugins install

.PHONY: install-man
install-man:
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock.1 $(DESTDIR)$(PREFIX)/share/man/man1/vlock.1
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-main.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-main.8
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-all.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-all.8
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-new.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-new.8
	$(INSTALL) -D -m 644 -o root -g $(ROOT_GROUP) man/vlock-nosysrq.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-nosysrq.8


### build rules ###

vlock: vlock.sh config.mk Makefile
	$(BOURNE_SHELL) -n $<
	sed \
		-e 's,%BOURNE_SHELL%,$(BOURNE_SHELL),' \
		-e 's,%PREFIX%,$(PREFIX),' \
		-e 's,%VLOCK_VERSION%,$(VLOCK_VERSION),' \
		$< > $@.tmp
	mv -f $@.tmp $@

override CFLAGS += -Isrc -DPREFIX="\"$(PREFIX)"\"

vlock-all: vlock-all.o
vlock-main: vlock-main.o prompt.o auth-$(AUTH_METHOD).o
vlock-new: vlock-new.o
vlock-nosysrq: vlock-nosysrq.o

auth-pam.o: auth-pam.c vlock.h
auth-shadow.o: auth-shadow.c vlock.h
prompt.o: prompt.c vlock.h
vlock-all.o: vlock-all.c vlock.h
vlock-main.o: vlock-main.c vlock.h
vlock-new.o: vlock-new.c vlock.h
vlock-nosysrq.o: vlock-nosysrq.c vlock.h
plugins.o: plugins.c vlock.h

ifneq ($(USE_ROOT_PASS),y)
vlock-main.o : override CFLAGS += -DNO_ROOT_PASS
endif

ifeq ($(AUTH_METHOD),pam)
vlock-main : override LDFLAGS += $(PAM_LIBS)
endif

ifeq ($(AUTH_METHOD),shadow)
vlock-main : override LDFLAGS += $(CRYPT_LIB)
endif

ifeq ($(USE_PLUGINS),y)
vlock-main: plugins.o
vlock-main : override LDFLAGS += $(DL_LIB)
vlock-main.o prompt.o : override CFLAGS += -DUSE_PLUGINS
endif

ifeq ($(USE_PAM),y)
vlock-nosysrq vlock-all : override LDFLAGS += $(PAM_LIBS)
vlock-nosysrq vlock-all : override CFLAGS += -DUSE_PAM
endif

.PHONY: clean
clean:
	$(RM) $(PROGRAMS) $(wildcard *.o)
	@$(MAKE) -C plugins clean

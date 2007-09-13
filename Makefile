# vlock makefile

include config.mk
include system.mk

VPATH = src

VLOCK_VERSION = 2.2 alpha1

PROGRAMS = vlock vlock-main

.PHONY: all
all: $(PROGRAMS)

ifeq ($(USE_PLUGINS),y)
all: plugins
endif

.PHONY: plugins
plugins: modules

.PHONY: modules
modules:
	@$(MAKE) -C modules

### installation rules ###

.PHONY: install
install: install-programs install-man

ifeq ($(USE_PLUGINS),y)
install: install-plugins
endif

.PHONY: install-programs
install-programs: $(PROGRAMS)
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 755 -o root -g $(ROOT_GROUP) vlock $(DESTDIR)$(PREFIX)/bin/vlock
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/sbin
	$(INSTALL) -m 4711 -o root -g $(ROOT_GROUP) vlock-main $(DESTDIR)$(PREFIX)/sbin/vlock-main

.PHONY: install-plugins
install-plugins: install-modules

.PHONY: install-modules
install-modules:
	@$(MAKE) -C modules install

.PHONY: install-man
install-man:
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/share/man/man1
	$(INSTALL) -m 644 -o root -g $(ROOT_GROUP) man/vlock.1 $(DESTDIR)$(PREFIX)/share/man/man1/vlock.1
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/share/man/man8
	$(INSTALL) -m 644 -o root -g $(ROOT_GROUP) man/vlock-main.8 $(DESTDIR)$(PREFIX)/share/man/man8/vlock-main.8


### build rules ###

vlock: vlock.sh config.mk Makefile
	$(BOURNE_SHELL) -n $<
	sed \
		-e 's,%BOURNE_SHELL%,$(BOURNE_SHELL),' \
		-e 's,%PREFIX%,$(PREFIX),' \
		-e 's,%VLOCK_VERSION%,$(VLOCK_VERSION),' \
		$< > $@.tmp
	mv -f $@.tmp $@

override CFLAGS += -Isrc -DPREFIX="\"$(PREFIX)\""
override CXXFLAGS += -Isrc -DPREFIX="\"$(PREFIX)\""

ifeq ($(DEBUG),y)
override CFLAGS += -g -O0
override CXXFLAGS += -g -O0
endif

vlock-main: vlock-main.o prompt.o auth-$(AUTH_METHOD).o

.INTERMEDIATE: vlock-main.o

auth-pam.o: auth-pam.c vlock.h
auth-shadow.o: auth-shadow.c vlock.h
prompt.o: prompt.c vlock.h
vlock-main.o: vlock-main.c vlock.h
plugins.o: plugins.cpp tsort.h list.h plugin.h plugins.h vlock.h
module.o: module.cpp module.h plugin.h
script.o: script.cpp script.h plugin.h
plugin.o: plugin.cpp plugin.h

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
vlock-main: plugins.o plugin.o module.o script.o
vlock-main : override LDFLAGS += $(DL_LIB) -lstdc++
vlock-main.o : override CFLAGS += -DUSE_PLUGINS
endif

.PHONY: clean
clean:
	$(RM) $(PROGRAMS) $(wildcard *.o)
	@$(MAKE) -C modules clean

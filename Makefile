# vlock makefile

include config.mk
include system.mk

VPATH = src

VLOCK_VERSION = 2.2 alpha1

PROGRAMS = vlock vlock-main

.PHONY: all
all: $(PROGRAMS)

.PHONY: debug
debug:
	@$(MAKE) DEBUG=y

ifeq ($(USE_PLUGINS),y)
all: plugins
endif

.PHONY: plugins
plugins: modules scripts

.PHONY: modules
modules:
	@$(MAKE) -C modules

.PHONY: scripts
scripts:
	@$(MAKE) -C scripts

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
install-plugins: install-modules install-scripts

.PHONY: install-modules
install-modules:
	@$(MAKE) -C modules install

.PHONY: install-scripts
install-scripts:
	@$(MAKE) -C scripts install

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
		-e 's,%VLOCK_USE_PLUGINS%,$(USE_PLUGINS),' \
		$< > $@.tmp
	mv -f $@.tmp $@

override CFLAGS += -Isrc

vlock-main: vlock-main.o prompt.o auth-$(AUTH_METHOD).o console_switch.o util.o

auth-pam.o: auth-pam.c prompt.h auth.h
auth-shadow.o: auth-shadow.c prompt.h auth.h
prompt.o: prompt.c prompt.h
vlock-main.o: vlock-main.c auth.h prompt.h util.h
plugins.o: plugins.c tsort.h plugin.h plugins.h list.h util.h
module.o : override CFLAGS += -DVLOCK_MODULE_DIR="\"$(VLOCK_MODULE_DIR)\""
module.o: module.c plugin.h list.h util.h
script.o : override CFLAGS += -DVLOCK_SCRIPT_DIR="\"$(VLOCK_SCRIPT_DIR)\""
script.o: script.c plugin.h process.h list.h util.h
plugin.o: plugin.c plugin.h list.h util.h
tsort.o: tsort.c tsort.h list.h
list.o: list.c list.h util.h
console_switch.o: console_switch.c console_switch.h
process.o: process.c process.h
util.o: util.c util.h

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
vlock-main: plugins.o plugin.o module.o process.o script.o tsort.o list.o
# -rdynamic is needed so that the all plugin can access the symbols from console_switch.o
vlock-main : override LDFLAGS += $(DL_LIB) -rdynamic
vlock-main.o : override CFLAGS += -DUSE_PLUGINS
vlock-main.o: plugins.h
endif

.PHONY: clean
clean:
	$(RM) $(PROGRAMS) $(wildcard *.o)
	@$(MAKE) -C modules clean
	@$(MAKE) -C scripts clean

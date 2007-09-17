include ../config.mk
include ../system.mk

MODULES += $(EXTRA_MODULES)

.PHONY: all
all: $(MODULES)

.PHONY: install
install: $(addprefix install-, $(MODULES))

MODULE_GROUP = $(ROOT_GROUP)
MODULE_MODE = 0755

install-new.so : MODULE_GROUP=$(VLOCK_GROUP)
install-new.so : MODULE_MODE=$(VLOCK_MODULE_MODE)
install-nosysrq.so : MODULE_GROUP=$(VLOCK_GROUP)
install-nosysrq.so : MODULE_MODE=$(VLOCK_MODULE_MODE)

# pseudo plugin
all.o : override CFLAGS += -I../src
all.o: all.c ../src/console_switch.h

.PHONY: install-%
install-% : MODULE_SCRIPT=$(<:.so=.sh)
install-%: %
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/lib/vlock/modules
	$(INSTALL) -m $(MODULE_MODE) -o root -g $(MODULE_GROUP) $< $(DESTDIR)$(PREFIX)/lib/vlock/modules/$<
	[ ! -e $(MODULE_SCRIPT) ] || \
	  $(INSTALL) -m 0644 -o root -g $(ROOT_GROUP) $(MODULE_SCRIPT) $(DESTDIR)$(PREFIX)/lib/vlock/modules/$(MODULE_SCRIPT)

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

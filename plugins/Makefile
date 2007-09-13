include ../config.mk
include ../system.mk

PLUGINS += $(EXTRA_PLUGINS)

.PHONY: all
all: $(PLUGINS)

.PHONY: install
install: $(addprefix install-, $(PLUGINS))

PLUGIN_GROUP = $(ROOT_GROUP)
PLUGIN_MODE = 0755

install-new.so : PLUGIN_GROUP=$(VLOCK_GROUP)
install-new.so : PLUGIN_MODE=$(VLOCK_PLUGIN_MODE)
install-nosysrq.so : PLUGIN_GROUP=$(VLOCK_GROUP)
install-nosysrq.so : PLUGIN_MODE=$(VLOCK_PLUGIN_MODE)

.PHONY: install-%
install-% : PLUGIN_SCRIPT=$(<:.so=.sh)
install-%: %
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/lib/vlock/modules
	$(INSTALL) -m $(PLUGIN_MODE) -o root -g $(PLUGIN_GROUP) $< $(DESTDIR)$(PREFIX)/lib/vlock/modules/$<
	[ ! -e $(PLUGIN_SCRIPT) ] || \
	  $(INSTALL) -m 0644 -o root -g $(ROOT_GROUP) $(PLUGIN_SCRIPT) $(DESTDIR)$(PREFIX)/lib/vlock/modules/$(PLUGIN_SCRIPT)

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

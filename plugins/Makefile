include ../config.mk
include ../system.mk

ifeq ($(origin PLUGINS),undefined)
ifeq ($(UNAME),Linux)
PLUGINS = vlock-all.so vlock-new.so vlock-nosysrq.so vlock-blank.so
else ifneq (,$(findstring FreeBSD,$(UNAME)))
PLUGINS = vlock-all.so vlock-new.so
endif
endif

.PHONY: all
all: $(PLUGINS)

.PHONY: install
install: $(addprefix install-, $(PLUGINS))

PLUGIN_GROUP = $(ROOT_GROUP)
PLUGIN_MODE = 0755

install-vlock-new.so : PLUGIN_GROUP=$(VLOCK_GROUP)
install-vlock-new.so : PLUGIN_MODE=$(VLOCK_PLUGIN_MODE)
install-vlock-nosysrq.so : PLUGIN_GROUP=$(VLOCK_GROUP)
install-vlock-nosysrq.so : PLUGIN_MODE=$(VLOCK_PLUGIN_MODE)

.PHONY: install-%
install-% : PLUGIN_SCRIPT=$(<:.so=.sh)
install-%: %
	$(INSTALL) -D -m $(PLUGIN_MODE) -o root -g $(PLUGIN_GROUP) $< $(DESTDIR)$(PREFIX)/lib/vlock/modules/$<
	[ ! -e $(PLUGIN_SCRIPT) ] || \
	  $(INSTALL) -D -m 0644 -o root -g $(ROOT_GROUP) $(PLUGIN_SCRIPT) $(DESTDIR)$(PREFIX)/lib/vlock/modules/$(PLUGIN_SCRIPT)

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

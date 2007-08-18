include ../config.mk

.PHONY: all
all: $(PLUGINS)

.PHONY: install
install: $(addprefix install-, $(PLUGINS))

PLUGIN_MODE = 0755

.PHONY: install-%
install-% : PLUGIN_SCRIPT=$(<:.so=.sh)
install-%: %
	$(INSTALL) -D -m $(PLUGIN_MODE) -o root -g $(ROOT_GROUP) $< $(DESTDIR)$(PREFIX)/lib/vlock/modules/$<
	[ ! -e $(PLUGIN_SCRIPT) ] || \
	  $(INSTALL) -D -m 0644 -o root -g $(ROOT_GROUP) $(PLUGIN_SCRIPT) $(DESTDIR)$(PREFIX)/lib/vlock/modules/$(PLUGIN_SCRIPT)

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

include ../config.mk

.PHONY: all
all: $(PLUGINS)

.PHONY: install
install: $(addprefix install-, $(PLUGINS))

.PHONY: install-%
install-%: %
	$(INSTALL) -D -m 4755 -o root -g $(ROOT_GROUP) $< $(DESTDIR)$(PREFIX)/lib/vlock/modules/$<

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

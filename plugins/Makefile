include ../config.mk

.PHONY: all
all: $(addprefix build-, $(PLUGINS))

.PHONY: build-%
.PHONY: install-%

build-vlock-all: vlock-all.so

install-vlock-all: build-vlock-all
	$(INSTALL) -D -m 4755 -o root -g $(ROOT_GROUP) vlock-all.so $(DESTDIR)$(PREFIX)/lib/vlock/modules/vlock-all.so

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

vlock-all.o: vlock-all.c

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

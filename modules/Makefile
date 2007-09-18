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
install-%: %
	$(MKDIR_P) $(DESTDIR)$(VLOCK_MODULE_DIR)
	$(INSTALL) -m $(MODULE_MODE) -o root -g $(MODULE_GROUP) $< $(DESTDIR)$(VLOCK_MODULE_DIR)/$<

%.so : LDFLAGS += -shared
%.so: %.o
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o) $(wildcard *.so)

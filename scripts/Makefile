include ../config.mk
include ../system.mk

.PHONY: all
all: $(SCRIPTS)

.PHONY: install
install: $(addprefix install-, $(SCRIPTS))

SCRIPT_GROUP = $(ROOT_GROUP)
SCRIPT_MODE = 0755

install-%: %
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/lib/vlock/scripts
	$(INSTALL) -m $(SCRIPT_MODE) -o root -g $(SCRIPT_GROUP) $< $(DESTDIR)$(PREFIX)/lib/vlock/scripts/$(<:.sh=)

.PHONY: clean
clean:

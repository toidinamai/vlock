include ../config.mk
include ../system.mk

.PHONY: all
all: $(SCRIPTS)

.PHONY: install
install: $(addprefix install-, $(SCRIPTS))

SCRIPT_GROUP = $(ROOT_GROUP)
SCRIPT_MODE = 0755

install-%.sh : SCRIPT_TARGET=$(<:.sh=)
install-%: %
	$(MKDIR_P) $(DESTDIR)$(VLOCK_SCRIPT_DIR)
	$(INSTALL) -m $(SCRIPT_MODE) -o root -g $(SCRIPT_GROUP) $< $(DESTDIR)$(VLOCK_SCRIPT_DIR)/$(SCRIPT_TARGET)

.PHONY: clean
clean:

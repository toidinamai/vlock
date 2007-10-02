include ../config.mk

.PHONY: all
all: $(SCRIPTS)

.PHONY: install
install: $(addprefix install-, $(SCRIPTS))

SCRIPT_GROUP = $(ROOT_GROUP)
SCRIPT_MODE = 0755

install-%.sh : SCRIPT_TARGET=$(<:.sh=)
install-%: %
	$(MKDIR_P) $(DESTDIR)$(SCRIPTDIR)
	$(INSTALL) -m $(SCRIPT_MODE) -o root -g $(SCRIPT_GROUP) $< $(DESTDIR)$(SCRIPTDIR)/$(SCRIPT_TARGET)

.PHONY: clean
clean:

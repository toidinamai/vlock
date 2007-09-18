ifndef ROOT_GROUP
ifeq ($(UNAME),FreeBSD)
ROOT_GROUP = wheel
else
ROOT_GROUP = root
endif
endif

ifeq ($(origin MODULES),undefined)
ifeq ($(UNAME),Linux)
MODULES = all.so new.so nosysrq.so
else ifneq (,$(findstring FreeBSD,$(UNAME)))
MODULES = all.so new.so
endif
endif

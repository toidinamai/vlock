ifndef ROOT_GROUP
ifeq ($(UNAME),FreeBSD)
ROOT_GROUP = wheel
else
ROOT_GROUP = root
endif
endif

ifndef VLOCK_GROUP
VLOCK_GROUP = $(ROOT_GROUP)
endif
ifndef VLOCK_MODULE_MODE
VLOCK_MODULE_MODE = 0750
endif

ifeq ($(origin MODULES),undefined)
ifeq ($(UNAME),Linux)
MODULES = all.so new.so nosysrq.so
else ifneq (,$(findstring FreeBSD,$(UNAME)))
MODULES = all.so new.so
endif
endif

ifeq ($(DEBUG),y)
CFLAGS += -g -O0
endif

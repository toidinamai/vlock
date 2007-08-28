ifndef ROOT_GROUP
ifeq ($(UNAME),FreeBSD)
ROOT_GROUP = wheel
else
ROOT_GROUP = root
endif
endif

ifndef VLOCK_GROUP
VLOCK_GROUP = $(ROOT_GROUP)
ifndef VLOCK_MODE
VLOCK_MODE = 4711
endif
else # VLOCK_GROUP is defined
ifndef VLOCK_MODE
VLOCK_MODE = 4710
endif
endif

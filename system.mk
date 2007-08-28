ifndef ROOT_GROUP
ifeq ($(UNAME),FreeBSD)
ROOT_GROUP = wheel
else
ROOT_GROUP = root
endif
endif

ifndef VLOCK_GROUP
VLOCK_GROUP = $(ROOT_GROUP)
ifndef VLOCK_PLUGIN_MODE
VLOCK_PLUGIN_MODE = 0755
endif
else # VLOCK_PLUGIN_GROUP is defined
ifndef VLOCK_PLUGIN_MODE
VLOCK_PLUGIN_MODE = 0750
endif
endif

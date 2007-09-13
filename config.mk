### configuration options ###

# operating system, determines some defaults
UNAME := $(shell uname)
# authentification method (pam or shadow)
AUTH_METHOD = pam
# use pam for permission checking
USE_PAM = n
# also prompt for the root password in adition to the user's
USE_ROOT_PASS = y
# enable plugins for vlock-main
USE_PLUGINS = y
# which plugins should be build, default is architecture dependent
# MODULES = 
EXTRA_MODULES =

# root's group, default is architecture dependent
ROOT_GROUP =

# group to install vlock-main with, defaults to ROOT_GROUP
VLOCK_GROUP =
# mode to install privileged plugins with, defaults to 0750 if VLOCK_GROUP
# is unset and 0755 otherwise
VLOCK_MODULE_MODE =

### paths ###

# installation prefix
PREFIX = /usr/local
# installation root
DESTDIR =

### programs ###

# shell to run vlock.sh with (only bash is known to work)
BOURNE_SHELL = /bin/sh
# C compiler
CC = gcc
# C++ compiler
CXX = g++
# gnu install
INSTALL = install
# linker
LD = ld
# mkdir
MKDIR_P = mkdir -p

### compiler and linker settings ###

# C compiler flags
CFLAGS = -O2 -Wall -W -pedantic -std=gnu99
# C++ compiler flags
CXXFLAGS = -O2 -Wall -W -pedantic
# linker flags
LDFLAGS = 
# linker flags needed for dlopen and friends
DL_LIB = -ldl
# linker flags needed for crypt
CRYPT_LIB = -lcrypt
# linker flags needed for pam
PAM_LIBS = $(DL_LIB) -lpam

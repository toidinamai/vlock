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
# which plugins should be build (default is architecture dependent)
# PLUGINS = 

# group to install vlock-all and vlock-nosysrq as
# defaults to 'root')
VLOCK_GROUP =
# mode to install vlock-all and vlock-nosysrq as
# defaults to 4711 if group is unset and 4710 otherwise
VLOCK_MODE =

# root's group (default is architecture dependent)
ROOT_GROUP =

### paths ###

# installation prefix
PREFIX = /usr/local
# installation root
DESTDIR =

### programs ###

# shell to run vlock.sh with (only bash is known to work)
BOURNE_SHELL = /usr/bin/env bash
# c compiler
CC = gcc
# gnu install
INSTALL = install
# linker
LD = ld

### compiler and linker settings ###

# c compiler flags
CFLAGS = -O2 -Wall -W -pedantic -std=gnu99
# linker flags
LDFLAGS = 
# linker flags needed for dlopen and friends
DL_LIB = -ldl
# linker flags needed for crypt
CRYPT_LIB = -lcrypt
# linker flags needed for pam
PAM_LIBS = $(DL_LIB) -lpam

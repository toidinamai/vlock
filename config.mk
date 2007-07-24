# use pam for authentification
USE_PAM = y
# use pam for permission checking
USE_PAM_PERM = n
# also prompt for the root password in adition to the user's
USE_ROOT_PASS = y
# shell to run vlock.sh with
BOURNE_SHELL = /bin/sh

VLOCK_GROUP = root
VLOCK_MODE = 4711

# installation base directory
PREFIX = /usr/local
DESTDIR =

CC = gcc
CFLAGS = -O2 -Wall -W -pedantic -std=gnu99
LDFLAGS = 

PAM_LIBS = -ldl -lpam -lpam_misc

INSTALL = install

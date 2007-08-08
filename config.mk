# authentification method (pam or shadow)
AUTH_METHOD = pam
# use pam for permission checking
USE_PAM = n
# also prompt for the root password in adition to the user's
USE_ROOT_PASS = y
# shell to run vlock.sh with
BOURNE_SHELL = /bin/sh

# group to install vlock-all and vlock-nosysrq as
# defaults to 'root')
VLOCK_GROUP =
# mode to install vlock-all and vlock-nosysrq as
# defaults to 4711 if group is unset and 4710 otherwise
VLOCK_MODE =

# installation base directory
PREFIX = /usr/local
DESTDIR =

CC = gcc
CFLAGS = -O2 -Wall -W -pedantic -std=gnu99
LDFLAGS = 

PAM_LIBS = -ldl -lpam -lpam_misc

INSTALL = install -D

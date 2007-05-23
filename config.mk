# should we build against pam?
USE_PAM = y
USE_ROOT_PASS = y
PREFIX = /usr/local
DESTDIR =

CC = gcc
CFLAGS = -O2
LDFLAGS = -ldl -lpam -lpam_misc

INSTALL = install

ifneq ($(USE_ROOT_PASS),y)
	CFLAGS += -DNO_ROOT_PASS
endif

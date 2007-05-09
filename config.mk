# should we build against pam?
USE_PAM = y
PREFIX = /usr/local

CC = gcc
CFLAGS = -O2
INSTALL = install

ifeq ($(USE_PAM),y)
	CFLAGS += -DUSE_PAM
	LDFLAGS += -ldl -lpam -lpam_misc
else
	CFLAGS += -DSHADOW_PWD
	LDFLAGS += -lcrypt
endif

/* vlock-all.c -- console grabbing plugin for vlock,
 *                the VT locking program for linux
 *
 * This program is copyright (C) 2007 Frank Benkstein, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <dlfcn.h>

#include "vlock_plugin.h"

struct console_switch_context
{
  void *handle;
  bool *locked;
  bool (*lock)(char **error);
  void (*unlock)(void);
};

struct console_switch_context *load_console_switch(void)
{
  struct console_switch_context *console_switch = malloc(sizeof *console_switch);

  if (console_switch == NULL)
    return NULL;

  console_switch->handle = dlopen(NULL, RTLD_GLOBAL | RTLD_NOW);


  if (console_switch->handle == NULL) {
    fprintf(stderr, "vlock-all: dlopen() failed %s\n", dlerror());
    free(console_switch);
    return NULL;
  }

  console_switch->locked = dlsym(console_switch->handle, "console_switch_locked");
  *(void **) &(console_switch->lock) = dlsym(console_switch->handle, "lock_console_switch");
  *(void **) &(console_switch->unlock) = dlsym(console_switch->handle, "unlock_console_switch");

  if (console_switch->lock == NULL || console_switch->unlock == NULL || console_switch->locked == NULL) {
    fprintf(stderr, "vlock-all: dlsym() failed %s\n", dlerror());
    dlclose(console_switch->handle);
    free(console_switch);
    return NULL;
  }

  return console_switch;
}

bool vlock_start(void **ctx_ptr)
{
  char *error = NULL;
  struct console_switch_context *console_switch = load_console_switch();

  if (console_switch == NULL) {
    fprintf(stderr, "vlock-all: could not load console_switch routines from main program\n");
    return false;
  }

  if (!console_switch->lock(&error)) {
    if (error != NULL) {
      fprintf(stderr, "vlock-all: %s\n", error);
      free(error);
    } else {
      fprintf(stderr, "vlock-all: could not disable console switching\n");
    }
  }

  *ctx_ptr = console_switch;
  return *console_switch->locked;
}

bool vlock_end(void **ctx_ptr)
{
  struct console_switch_context *console_switch = *ctx_ptr;

  if (console_switch != NULL) {
    console_switch->unlock();
    dlclose(console_switch->handle);
    free(console_switch);
  }

  return true;
}

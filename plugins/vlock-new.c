/* vlock-new.c -- console allocation routine for vlock,
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>

#ifdef __FreeBSD__
#include <sys/consio.h>
#else
#include <sys/vt.h>
#endif /* __FreeBSD__ */

#include "vlock_plugin.h"

const char *before[] = { "vlock-new", NULL };

/* name of the virtual console device */
#ifdef __FreeBSD__
#define CONSOLE "/dev/ttyv0"
#else
#define CONSOLE "/dev/tty0"
#endif
/* template for the device of a given virtual console */
#ifdef __FreeBSD__
#define VTNAME "/dev/ttyv%x"
#else
#define VTNAME "/dev/tty%d"
#endif

/* Get the currently active console from the given
 * console file descriptor.  Returns console number
 * (starting from 1) on success, -1 on error. */
#ifdef __FreeBSD__
static int get_active_console(int consfd) {
  int n;

  if (ioctl(consfd, VT_GETACTIVE, &n) == 0)
    return n;
  else
    return -1;
}
#else
static int get_active_console(int consfd) {
  struct vt_stat vtstate;

  /* get the virtual console status */
  if (ioctl(consfd, VT_GETSTATE, &vtstate) == 0)
    return vtstate.v_active;
  else
    return -1;
}
#endif

/* Get the device name for the given console number.
 * Returns the device name or NULL on error. */
static char *get_console_name(int n) {
  static char name[sizeof VTNAME + 2];
  size_t namelen;

  if (n <= 0)
    return NULL;

  /* format the virtual terminal filename from the number */
#ifdef __FreeBSD__
  namelen = snprintf(name, sizeof name, VTNAME, n-1);
#else
  namelen = snprintf(name, sizeof name, VTNAME, n);
#endif

  if (namelen > sizeof name) {
    fprintf(stderr, "vlock-new: virtual terminal number too large\n");
    return NULL;
  }
  else {
    return name;
  }
}

/* Change to the given console number using the given console
 * file descriptor. */
static int activate_console(int consfd, int vtno) {
  int c = ioctl(consfd, VT_ACTIVATE, vtno);

  if (c < 0)
    return c;
  else
    return ioctl(consfd, VT_WAITACTIVE, vtno);
}

struct new_console_context {
  int consfd;
  int old_vtno;
  int new_vtno;
  int saved_stdin;
  int saved_stdout;
  int saved_stderr;
};

/* Run vlock-all on a new console. */
int vlock_start(void **ctx_ptr) {
  struct new_console_context *ctx;
  int vtfd;
  char *vtname;

  /* allocate the context */
  if ((ctx = malloc(sizeof *ctx)) == NULL)
    return -1;

  /* try stdin first */
  ctx->consfd = dup(STDIN_FILENO);

  /* get the number of the currently active console */
  ctx->old_vtno = get_active_console(ctx->consfd);

  if (ctx->old_vtno < 0) {
    /* stdin is does not a virtual console */
    (void) close(ctx->consfd);

    /* XXX: add optional PAM check here */

    /* open the virtual console directly */
    if ((ctx->consfd = open(CONSOLE, O_RDWR)) < 0) {
      perror("vlock-new: cannot open virtual console");
      goto err;
    }

    /* get the number of the currently active console, again */
    ctx->old_vtno = get_active_console(ctx->consfd);

    if (ctx->old_vtno < 0) {
      perror("vlock-new: could not find out currently active console");
      goto err;
    }
  }

  /* get a free virtual terminal number */
  if (ioctl(ctx->consfd, VT_OPENQRY, &ctx->new_vtno) < 0)  {
    perror("vlock-new: could not find a free virtual terminal");
    goto err;
  }

  /* get the device name for the new virtual console */
  vtname = get_console_name(ctx->new_vtno);

  /* open the free virtual terminal */
  if ((vtfd = open(vtname, O_RDWR)) < 0) {
    perror("vlock-new: cannot open new console");
    goto err;
  }

  /* work around stupid X11 bug */
  if (getenv("DISPLAY") != NULL)
    sleep(1);

  /* switch to the virtual terminal */
  if (activate_console(ctx->consfd, ctx->new_vtno) < 0) {
    perror("vlock-new: could not activate new terminal");
    goto err;
  }

  /* save the stdio file descriptors */
  ctx->saved_stdin = dup(STDIN_FILENO);
  ctx->saved_stdout = dup(STDOUT_FILENO);
  ctx->saved_stderr = dup(STDERR_FILENO);

  /* redirect stdio to virtual terminal */
  (void) dup2(vtfd, STDIN_FILENO);
  (void) dup2(vtfd, STDOUT_FILENO);
  (void) dup2(vtfd, STDERR_FILENO);

#if 0
  /* make virtual terminal controlling tty of this process */
  if (ioctl(vtfd, TIOCSCTTY, 1) < 0) {
    perror("vlock-new: could not set controlling terminal");
    _exit(111);
  }
#endif

  /* close virtual terminal file descriptor */
  (void) close(vtfd);

  *ctx_ptr = ctx;
  return 0;

err:
  free(ctx);
  return -1;
}

int vlock_end(void **ctx_ptr) {
  struct new_console_context *ctx = *ctx_ptr;

  /* restore saved stdio filedescriptors */
  (void) dup2(ctx->saved_stdin, STDIN_FILENO);
  (void) dup2(ctx->saved_stdout, STDOUT_FILENO);
  (void) dup2(ctx->saved_stderr, STDERR_FILENO);

  /* switch back to former virtual terminal */
  if (activate_console(ctx->consfd, ctx->old_vtno) < 0)
    perror("vlock-new: could not activate previous console");

#ifndef __FreeBSD__
  /* deallocate virtual terminal */
  if (ioctl(ctx->consfd, VT_DISALLOCATE, ctx->new_vtno) < 0)
    perror("vlock-new: could not disallocate console");
#endif

  (void) close(ctx->consfd);

  return 0;
}

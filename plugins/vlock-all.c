/* vlock-all.c -- console grabbing routine for vlock,
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

#ifdef __FreeBSD__
#include <sys/consio.h>
#else
#include <sys/vt.h>
#endif /* __FreeBSD__ */

#include "vlock_plugin.h"

/* This handler is called by a signal whenever a user tries to
 * switch away from this virtual console. */
static void release_vt(int __attribute__((__unused__)) signum) {
  /* kernel is not allowed to switch */
  (void) ioctl(STDIN_FILENO, VT_RELDISP, 0);
}

/* This handler is called whenever a user switches to this
 * virtual console. */
static void acquire_vt(int __attribute__((__unused__)) signum) {
  /* acknowledge, this is a noop */
  (void) ioctl(STDIN_FILENO, VT_RELDISP, VT_ACKACQ);
}

int vlock_start(void **ctx_ptr) {
  struct vt_mode vtm, *ctx;
  struct sigaction sa;

  /* allocate the context */
  if ((ctx = malloc(sizeof *ctx)) == NULL)
    return -1;

  /* get the virtual console mode */
  if (ioctl(STDIN_FILENO, VT_GETMODE, &vtm) < 0) {
    if (errno == ENOTTY || errno == EINVAL)
      fprintf(stderr, "vlock-all: this terminal is not a virtual console\n");
    else
      perror("vlock-all: could not get virtual console mode");

    goto err;
  }

  /* save the virtual console mode */
  (void) memcpy(ctx, &vtm, sizeof vtm);

  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = release_vt;
  (void) sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  (void) sigaction(SIGUSR2, &sa, NULL);

  /* set terminal switching to be process governed */
  vtm.mode = VT_PROCESS;
  /* set terminal release signal, i.e. sent when switching away */
  vtm.relsig = SIGUSR1;
  /* set terminal acquire signal, i.e. sent when switching here */
  vtm.acqsig = SIGUSR2;
  /* set terminal free signal, not implemented on either FreeBSD or Linux */
  /* Linux ignores it but FreeBSD wants a valid signal number here */
  vtm.frsig = SIGHUP;

  /* set virtual console mode to be process governed
   * thus disabling console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, &vtm) < 0) {
    perror("vlock-all: could not set virtual console mode");
    goto err;
  }

  *ctx_ptr = ctx;
  return 0;

err:
  free(ctx);
  return -1;
}

int vlock_end(void **ctx_ptr) {
  struct vt_mode *ctx = *ctx_ptr;

  if (ctx != NULL)
    /* globally enable virtual console switching */
    if (ioctl(STDIN_FILENO, VT_SETMODE, ctx) < 0) {
      perror("vlock-all: could not restore console mode");
      return -1;
    }

  return 0;
}

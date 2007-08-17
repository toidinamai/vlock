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

void vlock_start(void **ctx_ptr) {
  struct vt_mode *ctx;
  struct sigaction sa;

  /* allocate the context */
  if ((ctx = malloc(sizeof *ctx)) == NULL)
    return;

  /* get the virtual console mode */
  if (ioctl(STDIN_FILENO, VT_GETMODE, ctx) < 0) {
    if (errno == ENOTTY || errno == EINVAL)
      fprintf(stderr, "vlock-all: this terminal is not a virtual console\n");
    else
      perror("vlock-all: could not get virtual console mode");

    goto err;
  }

  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = release_vt;
  (void) sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  (void) sigaction(SIGUSR2, &sa, NULL);

  /* back up current terminal mode */
  (void) memcpy(ctx, &vtmode, sizeof *ctx);
  /* set terminal switching to be process governed */
  ctx->mode = VT_PROCESS;
  /* set terminal release signal, i.e. sent when switching away */
  ctx->relsig = SIGUSR1;
  /* set terminal acquire signal, i.e. sent when switching here */
  ctx->acqsig = SIGUSR2;
  /* set terminal free signal, not implemented on either FreeBSD or Linux */
  /* Linux ignores it but FreeBSD wants a valid signal number here */
  ctx->frsig = SIGHUP;

  /* set virtual console mode to be process governed
   * thus disabling console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, ctx) < 0) {
    perror("vlock-all: could not set virtual console mode");
    goto err;

  *ctx_ptr = ctx;

err:
  free(ctx);
}

void vlock_end(struct vtmode *ctx) {
  if (ctx == NULL)
    return;

  /* globally enable virtual console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, ctx) < 0)
    perror("vlock-all: could not restore console mode");
}

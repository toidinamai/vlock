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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/consio.h>
#else
#include <sys/vt.h>
#endif /* __FreeBSD__, __FreeBSD_kernel__ */

#include "vlock.h"

/* This handler is called by a signal whenever a user tries to
 * switch away from this virtual console. */
void release_vt(int __attribute__((__unused__)) signum) {
  /* kernel is not allowed to switch */
  ioctl(STDIN_FILENO, VT_RELDISP, 0);
}

/* This handler is called whenever a user switches to this
 * virtual console. */
void acquire_vt(int __attribute__((__unused__)) signum) {
  /* acknowledge, this is a noop */
  ioctl(STDIN_FILENO, VT_RELDISP, VT_ACKACQ);
}

/* Disable console switching while running vlock-current. */
int main(void) {
  struct vt_mode vtmode, vtmode_bak;
  struct sigaction sa;
  int pid;
  int status;

  /* get the virtual console mode */
  if (ioctl(STDIN_FILENO, VT_GETMODE, &vtmode) < 0) {
    if (errno == ENOTTY || errno == EINVAL)
      fprintf(stderr, "vlock-all: this terminal is not a virtual console\n");
    else
      perror("vlock-all: could not get virtual console mode");

    exit (111);
  }

  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = release_vt;
  (void) sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  (void) sigaction(SIGUSR2, &sa, NULL);

  /* back up current terminal mode */
  vtmode_bak = vtmode;
  /* set terminal switching to be process governed */
  vtmode.mode = VT_PROCESS;
  /* set terminal release signal, i.e. sent when switching away */
  vtmode.relsig = SIGUSR1;
  /* set terminal acquire signal, i.e. sent when switching here */
  vtmode.acqsig = SIGUSR2;
  /* set terminal free signal, not implemented on either FreeBSD or Linux */
  /* Linux ignores it but FreeBSD wants a valid signal number here */
  vtmode.frsig = SIGHUP;

  /* set virtual console mode to be process governed
   * thus disabling console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, &vtmode) < 0) {
    perror("vlock-all: could not set virtual console mode");
    exit (111);
  }

  pid = fork();

  if (pid == 0) {
    /* child */

    /* drop privleges */
    (void) setuid(getuid());

    /* run child */
    execl(VLOCK_CURRENT, VLOCK_CURRENT, (char *) NULL);
    perror("vlock-all: exec of vlock-current failed");
    _exit(127);
  } else if (pid < 0) {
    perror("vlock-all: could not create child process");
  }

  if (pid > 0 && waitpid(pid, &status, 0) < 0) {
    perror("vlock-all: child process missing");
    pid = -1;
  }

  /* globally enable virtual console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, &vtmode_bak) < 0)
    perror("vlock-all: could not restore console mode");

  /* exit with the exit status of the child or 128+signal if it was killed */
  if (pid > 0) {
    if (WIFEXITED(status))
      exit (WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      exit (128+WTERMSIG(status));
  }

  return 0;
}

/* vlock-grab.c -- console grabbing routine for vlock,
 *                 the VT locking program for linux
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
#include <sys/vt.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

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

/* Run the program given by argv+1.  Console switching is forbidden
 * while the program is running. */
int main(void) {
  struct vt_mode vtmode;
  struct vt_mode vtmode_bak;
  int pid;
  int status;

  /* XXX: add optional PAM check here */

  /* get the virtual console mode */
  if (ioctl(STDIN_FILENO, VT_GETMODE, &vtmode) < 0) {
    if (errno == ENOTTY || errno == EINVAL)
      fprintf(stderr, "vlock-grab: this terminal is not a virtual console\n");
    else
      perror("vlock-grab: could not get virtual console mode");

    exit (111);
  }

  vtmode_bak = vtmode;
  vtmode.mode = VT_PROCESS;
  vtmode.relsig = SIGUSR1;
  vtmode.acqsig = SIGUSR2;

  /* set virtual console mode to be process governed
   * thus disabling console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, &vtmode) < 0) {
    perror("vlock-grab: could not set virtual console mode");
    exit (111);
  }

  if (signal(SIGUSR1, release_vt) == SIG_ERR
      || signal(SIGUSR2, acquire_vt) == SIG_ERR) {
    perror("vlock-grab: could not install signal handlers");
    exit (111);
  }

  pid = fork();

  if (pid == 0) {
    /* child */

    /* drop privleges */
    setuid(getuid());

    /* run child */
    execl(VLOCK_LOCK, VLOCK_LOCK, (char *) NULL);
    perror("vlock-grab: exec of vlock-lock failed");
    _exit(127);
  } else if (pid < 0) {
    perror("vlock-grab: could not create child process");
  }

  if (pid > 0 && waitpid(pid, &status, 0) < 0) {
    perror("vlock-grab: child process missing");
    pid = -1;
  }

  /* globally enable virtual console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, &vtmode_bak) < 0) {
    perror("vlock-grab: could not restore console mode");
  }

  /* exit with the exit status of the child or 128+signal if it was killed */
  if (pid > 0) {
    if (WIFEXITED(status)) {
      exit (WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      exit (128+WTERMSIG(status));
    }
  }

  return 0;
}

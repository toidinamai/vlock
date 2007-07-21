/* vlock-grab.c -- console grabbing routine for vlock,
 *                   the VT locking program for linux
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
#include <getopt.h>
#include <termios.h>
#include <signal.h>
#include <sys/vt.h>
#include <sys/kd.h>
#include <sys/ioctl.h>
#include "vlock.h"
#include "version.h"

#define VTNAME "/dev/tty%d"

/* Grab a new console and run the program given by argv+1 there.  Console
 * switching is locked as long as the program is running.  When the program
 * is finished, remove locking and switch back to former console. */
int main(int argc, char **argv) {
  int consfd = -1;
  struct vt_stat vtstat;
  int vtno;
  int vtfd;
  char vtname[sizeof VTNAME + 2];
  int pid;
  int status;

  if (argc < 2) {
    fprintf(stderr, "usage: %s child\n", *argv);
    exit (111);
  }

  /* open the current terminal */
  if ((consfd = open("/dev/tty", O_RDWR)) < 0) {
    perror("vlock: could not open /dev/tty");
    exit (1);
  }

  /* get the virtual console status */
  if (ioctl(consfd, VT_GETSTATE, &vtstat) < 0) {
    /* the current terminal does not belong to the virtual console */
    close(consfd);

    /* XXX: add a PAM check here */

    /* open the virtual console directly */
    if ((consfd = open("/dev/console", O_RDWR)) < 0) {
      perror("vlock: cannot open virtual console");
      exit (1);
    }

    /* get the virtual console status, again */
    if (ioctl(consfd, VT_GETSTATE, &vtstat) < 0) {
      perror("vlock: virtual console not a virtual console");
      exit (1);
    }
  }

  /* get a free virtual terminal number */
  if (ioctl(consfd, VT_OPENQRY, &vtno) < 0)  {
    perror("vlock: could not find a free virtual terminal");
    exit (1);
  }

  if (snprintf(vtname, sizeof vtname, VTNAME, vtno) > sizeof vtname) {
    fprintf(stderr, "virtual terminal number too large\n");
  }

  /* open the free virtual terminal */
  if ((vtfd = open(vtname, O_RDWR)) < 0) {
    perror("vlock: cannot open new console");
    exit (1);
  }

  /* switch to the virtual terminal */
  if (ioctl(consfd, VT_ACTIVATE, vtno) < 0
      || ioctl(consfd, VT_WAITACTIVE, vtno) < 0) {
    perror("vlock: could not activate new terminal");
    exit (1);
  }

  /* globally disable virtual console switching */
  if (ioctl(consfd, VT_LOCKSWITCH) < 0) {
    perror("vlock: could not disable console switching");
    exit (1);
  }

  pid = fork();

  if (pid < 0) {
    perror("vlock: could not create child process");
  } else if (pid == 0) {
    /* child */

    /* XXX: chown virtual terminal? */
    /* drop privleges */
    setuid(getuid());
    /* redirect stdio */
    dup2(vtfd, STDIN_FILENO);
    dup2(vtfd, STDOUT_FILENO);
    dup2(vtfd, STDERR_FILENO);
    close(vtfd);

    /* run child */
    execvp(*(argv+1), argv+1);
    perror("vlock: exec failed");
    _exit(127);
  }

  close(vtfd);
  waitpid(pid, &status, 0);

  /* globally enable virtual console switching */
  if (ioctl(consfd, VT_UNLOCKSWITCH) < 0) {
    perror("vlock: could not enable console switching");
    exit (1);
  }

  /* switch back to former virtual terminal */
  if (ioctl(consfd, VT_ACTIVATE, vtstat.v_active) < 0
      || ioctl(consfd, VT_WAITACTIVE, vtstat.v_active) < 0) {
    perror("vlock: could not activate old console");
  }

  /* deallocate virtual terminal */
  if (ioctl(consfd, VT_DISALLOCATE, vtno) < 0) {
    perror("vlock: could not disallocate console");
  }

  return 0;
}

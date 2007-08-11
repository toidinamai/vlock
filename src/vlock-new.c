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

#include "vlock.h"

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

/* Run vlock-all on a new console. */
int main(void) {
  int consfd = STDIN_FILENO;
  int old_vtno;
  int vtno;
  int vtfd;
  char *vtname;
  int pid = -1;
  int status;

  /* get the number of the currently active console */
  old_vtno = get_active_console(consfd);

  if (old_vtno < 0) {
    /* stdin is does not a virtual console */
    (void) close(consfd);

    /* XXX: add optional PAM check here */

    /* open the virtual console directly */
    if ((consfd = open(CONSOLE, O_RDWR)) < 0) {
      perror("vlock-new: cannot open virtual console");
      exit (111);
    }

    /* get the number of the currently active console, again */
    old_vtno = get_active_console(consfd);

    if (old_vtno < 0) {
      perror("vlock-new: could not find out currently active console");
      exit (111);
    }
  }

  /* get a free virtual terminal number */
  if (ioctl(consfd, VT_OPENQRY, &vtno) < 0)  {
    perror("vlock-new: could not find a free virtual terminal");
    exit (111);
  }

  /* get the device name for the new virtual console */
  vtname = get_console_name(vtno);

  /* open the free virtual terminal */
  if ((vtfd = open(vtname, O_RDWR)) < 0) {
    perror("vlock-new: cannot open new console");
    exit (111);
  }

  /* switch to the virtual terminal */
  if (ioctl(consfd, VT_ACTIVATE, vtno) < 0
      || ioctl(consfd, VT_WAITACTIVE, vtno) < 0) {
    perror("vlock-new: could not activate new terminal");
    exit (111);
  }

  pid = fork();

  if (pid == 0) {
    /* child */

    /* close the virtual console file descriptor */
    (void) close(consfd);

    /* drop privleges */
    (void) setuid(getuid());

    /* make this process a session leader */
    (void) setsid();

    /* make new virtual terminal controlling tty of this process */
    if (ioctl(vtfd, TIOCSCTTY, 1) < 0) {
      perror("vlock-new: could not set controlling terminal");
      _exit(111);
    }

    /* redirect stdio */
    dup2(vtfd, STDIN_FILENO);
    dup2(vtfd, STDOUT_FILENO);
    dup2(vtfd, STDERR_FILENO);
    close(vtfd);

    /* run child */
    execl(VLOCK_ALL, VLOCK_ALL, (char *) NULL);
    perror("vlock-new: exec of vlock-all failed");
    _exit(127);
  } else if (pid < 0) {
    perror("vlock-new: could not create child process");
  }

  /* close virtual terminal file descriptor */
  (void) close(vtfd);

  if (pid > 0 && waitpid(pid, &status, 0) < 0) {
    perror("vlock-new: child process missing");
    pid = -1;
  }

  /* switch back to former virtual terminal */
  if (ioctl(consfd, VT_ACTIVATE, old_vtno) < 0
      || ioctl(consfd, VT_WAITACTIVE, old_vtno) < 0)
    perror("vlock-new: could not activate previous console");

#ifndef __FreeBSD__
  /* deallocate virtual terminal */
  if (ioctl(consfd, VT_DISALLOCATE, vtno) < 0)
    perror("vlock-new: could not disallocate console");
#endif

  (void) close(consfd);

  /* exit with the exit status of the child or 128+signal if it was killed */
  if (pid > 0) {
    if (WIFEXITED(status))
      exit (WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      exit (128+WTERMSIG(status));
  }

  return 0;
}

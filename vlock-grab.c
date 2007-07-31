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
#include <sysexits.h>

/* Run the program given by argv+1.  Console switching is forbidden
 * while the program is running.
 *
 * CAP_SYS_TTY_CONFIG is needed for the locking to succeed.
 */
/* XXX: clean up exit codes */
int main(int argc, char **argv) {
  struct vt_stat vtstat;
  int pid;
  int status;

  if (argc < 2) {
    fprintf(stderr, "usage: %s child\n", *argv);
    exit (EX_USAGE);
  }

  /* XXX: add optional PAM check here */

  /* get the virtual console status */
  if (ioctl(STDIN_FILENO, VT_GETSTATE, &vtstat) < 0) {
    if (errno == ENOTTY || errno == EINVAL)
      fprintf(stderr, "vlock-grab: this terminal is not a virtual console\n");
    else
      perror("vlock-grab: could not get virtual console status");

    exit (EX_IOERR);
  }

  /* globally disable virtual console switching */
  if (ioctl(STDIN_FILENO, VT_LOCKSWITCH) < 0) {
    perror("vlock-grab: could not disable console switching");
    if (errno == EPERM)
      exit (EX_NOPERM);
    else
      exit (EX_IOERR);
  }

  pid = fork();

  if (pid == 0) {
    /* child */

    /* drop privleges */
    setuid(getuid());

    /* run child */
    execvp(*(argv+1), argv+1);
    perror("vlock-grab: exec failed");
    _exit(127);
  } else if (pid < 0) {
    perror("vlock-grab: could not create child process");
  }

  if (pid > 0 && waitpid(pid, &status, 0) < 0) {
    perror("vlock-grab: child process missing");
    pid = -1;
  }

  /* globally enable virtual console switching */
  if (ioctl(STDIN_FILENO, VT_UNLOCKSWITCH) < 0) {
    perror("vlock-grab: could not reenable console switching");
  }

  /* exit with the exit status of the child or 200+signal if
   * it was killed */
  if (pid > 0) {
    if (WIFEXITED(status)) {
      exit (WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      exit (128+WTERMSIG(status));
    }
  }

  return EX_OK;
}

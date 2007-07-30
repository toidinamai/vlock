/* vlock-grab.c -- sysrq protection routine for vlock,
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
#include <sys/wait.h>

#define SYSRQ_PATH "/proc/sys/kernel/sysrq"
#define SYSRQ_DISABLE_VALUE "0\n"

/* Run the program given by argv+1.  SysRQ keys are disabled while
 * as the program is running.
 *
 * CAP_SYS_TTY_CONFIG is needed for the locking to succeed.
 */
/* XXX: clean up exit codes */
int main(int argc, char **argv) {
  char sysrq[32];
  int pid;
  int status;
  FILE *f;

  if (argc < 2) {
    fprintf(stderr, "usage: %s child\n", *argv);
    exit (111);
  }

  /* XXX: add optional PAM check here */

  /* open the sysrq sysctl file for reading and writing */
  if ((f = fopen(SYSRQ_PATH, "r+")) == NULL) {
    perror("vlock-nosysrq: could not open '" SYSRQ_PATH "'");
    exit (1);
  }

  /* read the old value */
  if (fgets(sysrq, sizeof sysrq, f) == NULL) {
    perror("vlock-nosysrq: could not read from '" SYSRQ_PATH "'");
    exit (1);
  }

  /* check whether all was read */
  if (fgetc(f) != EOF) {
    fprintf(stderr, "vlock-nosysrq: sysrq buffer to small: %d\n", sizeof sysrq);
    exit (1);
  }

  /* seek to the start of and truncate the file */
  if (fseek(f, 0, SEEK_SET) < 0 || ftruncate(fileno(f), 0) < 0) {
    perror("vlock-nosysrq: could not truncate '" SYSRQ_PATH "'");
    exit (1);
  }

  /* disable sysrq */
  if (fputs(SYSRQ_DISABLE_VALUE, f) < 0 || fflush(f) < 0) {
    perror("vlock-nosysrq: could not write disable value to '" SYSRQ_PATH "'");
    exit (1);
  }

  pid = fork();

  if (pid == 0) {
    /* child */

    /* drop privleges */
    setuid(getuid());

    /* run child */
    execvp(*(argv+1), argv+1);
    perror("vlock-nosysrq: exec failed");
    _exit(127);
  } else if (pid < 0)
    perror("vlock-nosysrq: could not create child process");
  else if (waitpid(pid, &status, 0) < 0) {
    perror("vlock-nosysrq: child process missing");
    pid = -1;
  }

  /* seek to the start of the file */
  if (fseek(f, 0, SEEK_SET) < 0) {
    perror("vlock-nosysrq: could not seek to the start of '" SYSRQ_PATH "'");
    exit (1);
  }

  /* truncate the file */
  if (ftruncate(fileno(f), 0)) {
    perror("vlock-nosysrq: could not truncate '" SYSRQ_PATH "'");
    exit (1);
  }

  /* restore the old value */
  if (fputs(sysrq, f) < 0) {
    perror("vlock-nosysrq: could not old value to '" SYSRQ_PATH "'");
    exit (1);
  }

  /* exit with the exit status of the child or 200+signal if
   * it was killed */
  if (pid > 0) {
    if (WIFEXITED(status)) {
      exit (WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      exit (200+WTERMSIG(status));
    }
  }

  return 0;
}

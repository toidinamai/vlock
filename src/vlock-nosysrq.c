/* vlock-nosysrq.c -- sysrq protection routine for vlock,
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

#include "vlock.h"

#define SYSRQ_PATH "/proc/sys/kernel/sysrq"
#define SYSRQ_DISABLE_VALUE "0\n"

/* Run the program given by argv+1.  SysRQ keys are disabled while
 * as the program is running.
 *
 * CAP_SYS_TTY_CONFIG is needed for the locking to succeed.
 */
int main(void) {
  char sysrq[32];
  int pid;
  int status;
  FILE *f;

  /* XXX: add optional PAM check here */

  /* open the sysrq sysctl file for reading and writing */
  if ((f = fopen(SYSRQ_PATH, "r+")) == NULL) {
    perror("vlock-nosysrq: could not open '" SYSRQ_PATH "'");
    exit (111);
  }

  /* read the old value */
  if (fgets(sysrq, sizeof sysrq, f) == NULL) {
    perror("vlock-nosysrq: could not read from '" SYSRQ_PATH "'");
    exit (111);
  }

  /* check whether all was read */
  if (fgetc(f) != EOF) {
    fprintf(stderr, "vlock-nosysrq: sysrq buffer to small: %d\n", sizeof sysrq);
    exit (111);
  }

  /* disable sysrq */
  if (fseek(f, 0, SEEK_SET) < 0
      || ftruncate(fileno(f), 0) < 0
      || fputs(SYSRQ_DISABLE_VALUE, f) < 0
      || fflush(f) < 0) {
    perror("vlock-nosysrq: could not write disable value to '" SYSRQ_PATH "'");
    exit (111);
  }

  pid = fork();

  if (pid == 0) {
    /* child */

    /* close sysrq file */
    fclose(f);

    /* drop privleges */
    setuid(getuid());

    /* run child */
    if (getenv("VLOCK_NEW") != NULL) {
      execl(VLOCK_NEW, VLOCK_NEW, (char *) NULL);
      perror("vlock-nosysrq: exec of vlock-new failed");
    } else {
      execl(VLOCK_ALL, VLOCK_ALL, (char *) NULL);
      perror("vlock-nosysrq: exec of vlock-all failed");
    }
    _exit(127);
  } else if (pid < 0)
    perror("vlock-nosysrq: could not create child process");

  if (pid > 0 && waitpid(pid, &status, 0) < 0) {
    perror("vlock-nosysrq: child process missing");
    pid = -1;
  }

  if (fseek(f, 0, SEEK_SET) < 0
      || ftruncate(fileno(f), 0) < 0
      || fputs(sysrq, f) < 0
      || fflush(f) < 0
      ) {
    perror("vlock-nosysrq: could not restore old value to '" SYSRQ_PATH "'");
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

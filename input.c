/* input.c -- password getting module for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software, which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

/* RCS log:
 * $Log: input.c,v $
 * Revision 1.1  1994/03/15  18:27:33  johnsonm
 * Initial revision
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include "vlock.h"


static char rcsid[] = "$Id: input.c,v 1.2 1994/03/16 20:12:06 johnsonm Exp $";


/* get_password() should fork a new process to get the password. */
/* get_password() should return right away, but the new process  */
/* should not exit until it gets a proper password.              */

/* size of input buffer.  Shouldn't probably be very big.  Try 1... */
#define INBUFSIZE 1

void get_password(void) {

  pid_t pid;
  static char inbuf[INBUFSIZE];

  if ((pid = fork()) < 0) {
    perror("fork failed");
    exit(1);
  } else {
    if (pid > 0) {
      /* Parent */
      return; /* to wait for signals, including SIGCHLD from this process... */
    } else {
      /* Child */
      /* we need to ignore SIGCHLD now... */
      ignore_sigchld();
      printf("Please enter the password to unlock.\n");
      printf("%s's password:", getpwuid(getuid())->pw_name);
      fflush(stdout);

      /* set the terminal characteristics to not echo, etc... */
      set_terminal();

      /* Should get password here... */
      read(STDIN_FILENO, &inbuf, INBUFSIZE);

      /* reset the terminal characteristics to echo again before exiting */
      restore_terminal();
    }
  }
  fprintf(stderr, "exiting second process\n");
  fflush(stderr);
  exit(0);

}

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
 * $Log$
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "vlock.h"


static char rcsid[] = "$Id: input.c,v 1.1 1994/03/15 18:27:33 johnsonm Exp $";


/* get_password() should fork a new process to get the password. */
/* get_password() should return right away, but the new process  */
/* should not exit until it gets a proper password.              */

void get_password(void) {

  pid_t pid;

  if ((pid = fork()) < 0) {
    restore_terminal();
    perror("fork failed");
    exit(1);
  } else
    if (pid > 0)
      /* Parent */
      return; /* to wait for signals, including SIGCHLD from this process... */
    else
      /* Child */
      getchar(); /* should really get password here... */
  exit(0);

}

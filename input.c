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
 * Revision 1.2  1994/03/16  20:12:06  johnsonm
 * Now almost working.  Need to get signals straightened out for
 * second process.
 *
 * Revision 1.1  1994/03/15  18:27:33  johnsonm
 * Initial revision
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include "vlock.h"


static char rcsid[] = "$Id: input.c,v 1.3 1994/03/19 14:24:33 johnsonm Exp $";


void synch(void) {
 return;
}


/* get_password() should not return until a correct password has been */
/* entered. */

/* size of input buffer. */
#define INBUFSIZE 40

void get_password(void) {

  int c, i;
  pid_t pid;
  static char inbuf[INBUFSIZE];
  char *inptr;
  static struct passwd pwd;

  do {
    printf("Please enter the password to unlock.\n");
    printf("%s's password:", getpwuid(getuid())->pw_name);
    fflush(stdout);

    /* set the terminal characteristics to not echo, etc... */
    set_terminal();

    /* get password from user -- fgets() doesn't seem to work,
       probably because we have screwed with terminal settings */
    while (c = read(STDIN_FILENO, &inbuf[i], 1)) {
      synch(); /* possible compiler bug... */
      if (c > 0) {
	if (i+c >= INBUFSIZE) {
	  i = 0;
	  continue;
	}
	if ((inbuf[i+c-1] == '\n') || (inbuf[i+c-1] == '\r')) {
	  inbuf[i+c-1] == '\000';
	  break; /* done reading password */
	}
	else
	  i += c; /* read more */
      } else
	if (c == 0) {
	  i = 0;
	}
    }

    /* get rid of return or newline */
    if(inptr = strchr(inbuf, '\r') || (inptr = strchr(inbuf, '\n')))
      *inptr = '\000';

    /* should get password from /etc/password and encrypt user-provided
       one and see if they match */

  } while (0); /* this should strcmp the encrypted strings */

  /* reset the terminal characteristics to echo again before exiting */
  restore_terminal();
  printf("\n");
  
}

/* signals.c -- signal handline routine for vlock
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
 */


#include <stdio.h>
#include <unistd.h>
#include <linux/vt.h>
#include "vlock.h"


static char rcsid[] = "$Id: signals.c,v 1.1 1994/03/13 16:28:16 johnsonm Exp $";



/* In release_vt() and acquire_vt90, anything which is done in
 * release_vt() must be undone in acquire_vt().
 */


/* This is called by a signal whenever a user tries to change the VC
   with a ALT-Fn key */
void release_vt(int signo) {
  printf("here\n");
  if (!o_lock_all) {
    ioctl(STDIN_FILENO, VT_RELDISP, 1); /* kernel is allowed to switch */
  }
  else
    ioctl(STDIN_FILENO, VT_RELDISP, 0); /* kernel is not allowed to switch */
}


/* This is called whenever a user switches to that VC */
void acquire_vt(int signo) {
  /* This call is not currently required under Linux, but it won't hurt,
     either... */
  ioctl(STDIN_FILENO, VT_RELDISP, VT_ACKACQ);
}


void mask_signals(void) {
/* We don't want to get any job control signals */

}

void restore_signals(void) {

}

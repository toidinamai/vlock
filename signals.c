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
 * $Log: signals.c,v $
 * Revision 1.1  1994/03/13  16:28:16  johnsonm
 * Initial revision
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <linux/vt.h>
#include "vlock.h"


static char rcsid[] = "$Id: signals.c,v 1.2 1994/03/13 17:27:44 johnsonm Exp $";



/* In release_vt() and acquire_vt(), anything which is done in
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



static sigset_t osig;

void mask_signals(void) {

  static sigset_t sig;
  static struct sigaction sa;

  /* We don't want to get any job control signals (or others...). */
  sigemptyset(&sig);
  sigaddset(&sig, SIGUSR1);
  sigaddset(&sig, SIGUSR2);
  sigprocmask(SIG_SETMASK, &sig, &osig);

  /* we set SIGUSR{1,2} to point to *_vt() above */
  sigemptyset(&(sa.sa_mask));
  sa.sa_flags = 0;
  sa.sa_handler = release_vt;
  sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  sigaction(SIGUSR2, &sa, NULL);

}

void restore_signals(void) {

  sigprocmask(SIG_SETMASK, &osig, NULL);

}

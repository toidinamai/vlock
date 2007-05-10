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
 * Revision 1.5  1994/03/19  14:28:18  johnsonm
 * Removed support for silly two-process idea.  signal mask no longer
 * inverse of what I want.  (I may have fixed this in an earlier
 * version, and forgotten to log it.)
 *
 * Revision 1.4  1994/03/16  20:11:04  johnsonm
 * It mostly works -- child can still be killed by sigchld, which
 * propogates to parent, which kills lock program.  However, locking
 * all VC's does work if there is no other way to log on...
 *
 * Revision 1.3  1994/03/15  18:27:33  johnsonm
 * Added a few more signals, in preparation for two-process model.
 *
 * Revision 1.2  1994/03/13  17:27:44  johnsonm
 * Now using SIGUSR{1,2} correctly with release_vt() and acquire_vt() to
 * keep from switching VC's.
 *
 * Revision 1.1  1994/03/13  16:28:16  johnsonm
 * Initial revision
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <waitflags.h>
#include <linux/vt.h>
#include "vlock.h"


static char rcsid[] = "$Id: signals.c,v 1.6 1994/03/23 17:01:25 johnsonm Exp $";



/* In release_vt() and acquire_vt(), anything which is done in
 * release_vt() must be undone in acquire_vt().  Right now, that's
 * not much...
 */


/* This is called by a signal whenever a user tries to change the VC
   with a ALT-Fn key */
void release_vt(int signo) {
  if (!o_lock_all)
    ioctl(vfd, VT_RELDISP, 1); /* kernel is allowed to switch */
  else
    ioctl(vfd, VT_RELDISP, 0); /* kernel is not allowed to switch */
}


/* This is called whenever a user switches to that VC */
void acquire_vt(int signo) {
  /* This call is not currently required under Linux, but it won't hurt,
     either... */
  ioctl(vfd, VT_RELDISP, VT_ACKACQ);
}



void signal_ignorer(int signo) {
  return;
}




static sigset_t osig; /* for both mask_signals() and restore_signals() */

void mask_signals(void) {

  static sigset_t sig;
  static struct sigaction sa;

  /* We don't want to get any signals we don't have to, but there are   */
  /* several we must get.  I don't know whether to take the current     */
  /* signal mask and change it or to do a sigfillset and go from there. */
  /* The code should handle either, I think. */
  sigprocmask(SIG_SETMASK, NULL, &sig);
  sigdelset(&sig, SIGUSR1);
  sigdelset(&sig, SIGUSR2);
  sigaddset(&sig, SIGTSTP);
  sigaddset(&sig, SIGTTIN);
  sigaddset(&sig, SIGTTOU);
  sigaddset(&sig, SIGHUP);
  sigaddset(&sig, SIGCHLD);
  sigaddset(&sig, SIGQUIT);
  sigprocmask(SIG_SETMASK, &sig, &osig);
  

  /* we set SIGUSR{1,2} to point to *_vt() above */
  sigemptyset(&(sa.sa_mask));
  sa.sa_flags = 0;
  sa.sa_handler = release_vt;
  sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  sigaction(SIGUSR2, &sa, NULL);

  /* Need to handle some signals so that we don't get killed by them */
  sa.sa_handler = signal_ignorer;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
}



void restore_signals(void) {

  /* This probably isn't useful, but I'm including it anyway... */
  /* It might become useful later. */
  sigprocmask(SIG_SETMASK, &osig, NULL);


}

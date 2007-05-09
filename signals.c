/* signals.c -- signal handline routine for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/vt.h>
#include "vlock.h"


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



void set_signal_mask(int save) {

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
  sigaddset(&sig, SIGINT);
  if (save)
    sigprocmask(SIG_SETMASK, &sig, &osig);
  else
    sigprocmask(SIG_SETMASK, &sig, NULL);

  /* we set SIGUSR{1,2} to point to *_vt() above */
  sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = release_vt;
  sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  sigaction(SIGUSR2, &sa, NULL);

  /* Need to handle some signals so that we don't get killed by them */
  sa.sa_handler = signal_ignorer;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);
}





void mask_signals(void) {

  set_signal_mask(1);  /* set the signal masks and handlers and save */

}


void restore_signals(void) {

  /* This probably isn't useful, but I'm including it anyway... */
  /* It might become useful later, specifically if someone wants
     to include this code within another program. */
  sigprocmask(SIG_SETMASK, &osig, NULL);


}

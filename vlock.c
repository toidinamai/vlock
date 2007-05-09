/* vlock.c -- main routine for vlock, the VT locking program for linux
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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <termios.h>
#include <signal.h>
#include <sys/vt.h>
#include <sys/kd.h>
#include <sys/ioctl.h>
#include "vlock.h"
#include "version.h"


/* Option globals */
  /* This determines whether the default behavior is to lock only the */
  /* current VT or all of them.  0 means current, 1 means all. */
  int o_lock_all = 0;

  /* This determines whether the kernel.sysrq variable should be disabled */
  /* while the VT is locked. This implies that all VTs should be locked. */
  int o_disable_sysrq = 0;

/* Other globals */
  struct vt_mode ovtm;
  struct termios oterm;
  int vfd;
  int is_vt;

int main(int argc, char **argv) {

  static struct option long_options[] = { /* For parsing long arguments */
    {"current", no_argument, &o_lock_all, 0},
    {"all", no_argument, &o_lock_all, 1},
    {"disable-sysrq", no_argument, &o_disable_sysrq, 1},
    {"version", no_argument, 0, O_VERSION},
    {"help", no_argument, 0, O_HELP},
    {0, 0, 0, 0},
  };
  int option_index; /* Unused */
  int c;
  struct vt_mode vtm;

  /* First we parse all the command line arguments */
  while ((c = getopt_long(argc, argv, "acsvh",
			  long_options, &option_index)) != -1) {
    switch(c) {
    case 'c':
      o_lock_all = 0;
      break;
    case 'a':
      o_lock_all = 1;
      break;
    case 's':
      o_disable_sysrq = 1;
      break;
    case 'v':
    case O_VERSION:
      fprintf(stderr, VERSION);
      exit(0);
      break;
    case 'h':
    case O_HELP:
      print_help(0);
      break;
    case '?':
      print_help(1);
      break;
    }
  }

  /* Now we have parsed the options, and can get on with life */

  /* Get the user's and root's encrypted passwords.  This needs
     to run as root when using shadow passwords, but will drop
     root privileges as soon as they are no longer needed.  */
  init_passwords();

  if ((vfd = open("/dev/tty", O_RDWR)) < 0) {
    perror("vlock: could not open /dev/tty");
    exit (1);
  }

  /* First we will set process control of VC switching; if this fails, */
  /* then we know that we aren't on a VC, and will print a warning message */
  /* If it doesn't fail, it gets the current VT status... */
  c = ioctl(vfd, VT_GETMODE, &vtm);
  if (c < 0) {
    fprintf(stderr, " *** This tty is not a VC (virtual console). ***\n"
	    " *** It may not be securely locked. ***\n\n");
    is_vt = 0;
    o_lock_all = 0;
  } else {
    is_vt = 1;
  }

  /* Now set the signals so we can't be summarily executed or stopped, */
  /* and handle SIGUSR{1,2} and SIGCHLD */
  mask_signals();

  if (o_lock_all && o_disable_sysrq)
    /* disable the sysrq keys so we can't be killed through SAK or other keys */
    mask_sysrq();

  if (is_vt) {
    ovtm = vtm; /* Keep a copy around to restore at appropriate times */
    vtm.mode = VT_PROCESS;
    vtm.relsig = SIGUSR1; /* handled by release_vt() */
    vtm.acqsig = SIGUSR2; /* handled by acquire_vt() */
    ioctl(vfd, VT_SETMODE, &vtm);
  }

  /* get_password() sets the terminal characteristics and does not */
  /* return until the correct password has been read.              */
  get_password();

  /* vt status was restored in restore_terminal() already... */

  /* we should really return something... */
  return (0);

}


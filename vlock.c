/* vlock.c -- main routine for vlock, the VT locking program for linux
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
 * $Log: vlock.c,v $
 * Revision 1.7  1994/03/19  17:54:42  johnsonm
 * Moved printing the lock message to get_password.  Added a return
 * code again.
 *
 * Revision 1.6  1994/03/19  14:36:08  johnsonm
 * Made a better explanation for when the --all or -a flag is chosen.
 *
 * Revision 1.5  1994/03/19  14:25:16  johnsonm
 * Removed silly two-process model.  Must have been half asleep when
 * I came up with that idea.  vlock works now.
 *
 * Revision 1.4  1994/03/16  20:14:06  johnsonm
 * Cleaned up, putting most real work into child functions, which
 * cleaned up the interface.  The whole program is almost all
 * working now.
 *
 * Revision 1.3  1994/03/15  18:27:33  johnsonm
 * Moved terminal handling stuff into terminal.c, and changed the
 * end to support a two-process model for async I/O.
 *
 * Revision 1.2  1994/03/13  17:28:56  johnsonm
 * Now using SIGUSR{1,2} correctly to announce VC switches.  Fixed a few
 * other minor bugs.
 *
 * Revision 1.1  1994/03/13  16:28:16  johnsonm
 * Initial revision
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <termios.h>
#include <signal.h>
#include <sys/vt.h>
#include <sys/kd.h>
#include <linux/keyboard.h>
#include "vlock.h"
#include "version.h"


static char rcsid[] = "$Id: vlock.c,v 1.8 1994/03/20 11:21:20 johnsonm Exp $";

/* Option globals */
  /* This determines whether the default behavior is to lock only the */
  /* current VT or all of them.  0 means current, 1 means all. */
  int o_lock_all = 0;
  /* Pattern lists are not yet really supported, but we'll put in the */
  /* infrastructure for when they are. */
  Pattern *o_pattern_list = (Pattern *)0;

/* Other globals */
  struct vt_mode ovtm;
  struct termios oterm;
  int vfd;

int main(int argc, char **argv) {

  static struct option long_options[] = { /* For parsing long arguments */
    {"current", 0, &o_lock_all, 0},
    {"all", 0, &o_lock_all, 1},
    {"pattern", required_argument, 0, O_PATTERN},
    {"version", no_argument, 0, O_VERSION},
    {"help", no_argument, 0, O_HELP},
    {0, 0, 0, 0},
  };
  int option_index; /* Unused */
  int c;
  struct vt_mode vtm;

  /* First we parse all the command line arguments */
  while ((c = getopt_long(argc, argv, "acvhp:",
			  long_options, &option_index)) != -1) {
    switch(c) {
    case 'c':
      o_lock_all = 0;
      break;
    case 'a':
      o_lock_all = 1;
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
    case 'p':
    case O_PATTERN:
      fprintf(stderr, "The pattern list option is not yet supported.\n");
      break;
    case '?':
      print_help(1);
      break;
    }
  }

  /* Now we have parsed the options, and can get on with life */
  if (vfd = open("/dev/console", O_RDWR) < 0) {
    perror("vlock: could not open /dev/console");
    exit (1);
  }

  /* First we will set process control of VC switching; if this fails, */
  /* then we know that we aren't on a VC, and will print a message and */
  /* exit.   If it doesn't fail, it gets the current VT status... */
  c = ioctl(vfd, VT_GETMODE, &vtm);
  if (c < 0) {
    fprintf(stderr, "This tty is not a VC (virtual console), and cannot be locked.\n\n");
    print_help(1);
  }

  /* Now set the signals so we can't be summarily executed or stopped, */
  /* and handle SIGUSR{1,2} and SIGCHLD */
  mask_signals();

  ovtm = vtm; /* Keep a copy around to restore at appropriate times */
  vtm.mode = VT_PROCESS;
  vtm.relsig = SIGUSR1; /* handled by release_vt() */
  vtm.acqsig = SIGUSR2; /* handled by acquire_vt() */
  ioctl(vfd, VT_SETMODE, &vtm);

  /* get_password() sets the terminal characteristics and does not */
  /* return until the correct password has been read.              */
  get_password();

  /* we should really return something... */
  return (0);

}

/*
 * Local Variables: ***
 * mode:C ***
 * eval:(turn-on-auto-fill) ***
 * End: ***
 */

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
 * $Log$
 */


#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <pwd.h>
#include <sys/vt.h>
#include <sys/kd.h>
#include <linux/keyboard.h>
#include "vlock.h"
#include "version.h"


static char rcsid[] = "$Id: vlock.c,v 1.1 1994/03/13 16:28:16 johnsonm Exp $";

/* Option globals */
  /* This determines whether the default behavior is to lock only the */
  /* current VT or all of them.  0 means current, 1 means all. */
  int o_lock_all = 0;
  /* Pattern lists are not yet really supported, but we'll put in the */
  /* infrastructure for when they are. */
  Pattern *o_pattern_list = (Pattern *)0;

/* Other globals */
  struct vt_mode ovtm;

int main(int argc, char **argv) {

  static struct option long_options[] = { /* For parsing long arguments */
    {"current", 0, &o_lock_all, 0},
    {"all", 0, &o_lock_all, 1},
    {"pattern", required_argument, 0, O_PATTERN},
    {"version", required_argument, 0, O_VERSION},
    {"help", required_argument, 0, O_HELP},
    {0, 0, 0, 0},
  };
  int option_index; /* Unused */
  int c;
  struct vt_mode vtm;
  struct termios oterm, term;

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
  /* First we will set process control of VC switching; if this fails, */
  /* then we know that we aren't on a VC, and will print a message and */
  /* exit. */
  c = ioctl(STDOUT_FILENO, VT_GETMODE, &vtm);
  if (c < 0) {
    fprintf(stderr, "This tty is not a VC (virtual console), and cannot be locked.\n\n");
    print_help(1);
  }
  ovtm = vtm; /* Keep a copy around to restore at appropriate times */
  vtm.mode = VT_PROCESS;
  vtm.relsig = release_vt;
  vtm.acqsig = acquire_vt;
  ioctl(STDOUT_FILENO, VT_SETMODE, &vtm);

  /* Now set the signals so we can't be summarily executed or stopped */
  mask_signals();

  printf("Your TTY is now locked.  Please enter the password to unlock.\n");
  printf("%s's password:", getpwuid(getuid())->pw_name);

  tcgetattr(STDIN_FILENO, &oterm);
  term=oterm;
  term.c_iflag &= !BRKINT;
  term.c_iflag |= IGNBRK;
  term.c_lflag &= !(ECHO | ECHOCTL | ISIG);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);

  getchar();

  printf("\n");

  /* restore the VT settings before exiting */
  ioctl(STDOUT_FILENO, VT_SETMODE, &ovtm);
  tcsetattr(STDIN_FILENO, TCSANOW, &oterm);

}

/*
 * Local Variables: ***
 * mode:C ***
 * eval:(turn-on-auto-fill) ***
 * End: ***
 */

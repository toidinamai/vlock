/* terminal.c -- set and restore the terminal for vlock
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
#include <termios.h>
#include <sys/vt.h>
#include "vlock.h"


static char rcsid[] = "$Id: terminal.c,v 1.1 1994/03/15 18:27:33 johnsonm Exp $";


void set_terminal(void) {

  struct termios term;

  tcgetattr(STDIN_FILENO, &oterm);
  term=oterm;
  term.c_iflag &= !BRKINT;
  term.c_iflag |= IGNBRK;
  term.c_lflag &= !(ECHO | ECHOCTL | ISIG);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);

}



void restore_terminal(void) {

    ioctl(vfd, VT_SETMODE, &ovtm);
    tcsetattr(STDIN_FILENO, TCSANOW, &oterm);
    restore_signals();
    printf("\n");

}


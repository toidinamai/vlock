/* terminal.c -- set and restore the terminal for vlock
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
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/vt.h>
#include "vlock.h"


void set_terminal(int print) {

  static struct termios term;
  static int already=0;

  if (!already) {
    tcgetattr(STDIN_FILENO, &oterm);
    term=oterm;
    term.c_iflag &= ~BRKINT;
    term.c_iflag |= IGNBRK;
    term.c_lflag &= ~ISIG;
    already=1;
  }
  if (print)
    term.c_lflag |= (ECHO & ECHOCTL);
  else
    term.c_lflag &= ~(ECHO | ECHOCTL);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);

}



void restore_terminal(void) {

  if (is_vt) {
    ioctl(vfd, VT_SETMODE, &ovtm);
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &oterm);

}


/* terminal.c -- set and restore the terminal for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

/* RCS log:
 * $Log: terminal.c,v $
 * Revision 1.6  1994/07/03 13:10:06  johnsonm
 * Allow setting terminal state back and forth on echo, but keep ignoring sig,
 *   based on an argument.
 *
 * Revision 1.5  1994/07/03  12:31:26  johnsonm
 * Don't restore signals when restoring the terminal state!  Let other code
 *   do that later, when the correct password has been entered.
 *
 * Revision 1.4  1994/07/03  12:25:26  johnsonm
 * Only get old terminal characteristics the first time.
 *
 * Revision 1.3  1994/03/23  17:01:01  johnsonm
 * Added support for non-vt ttys.
 *
 * Revision 1.2  1994/03/17  18:22:08  johnsonm
 * Removed spurious printf.
 *
 * Revision 1.1  1994/03/15  18:27:33  johnsonm
 * Initial revision
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/vt.h>
#include "vlock.h"


static char rcsid[] = "$Id: terminal.c,v 1.7 1996/05/17 02:50:54 johnsonm Exp $";


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


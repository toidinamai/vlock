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
#include <sys/vt.h>
#include "vlock.h"


static char rcsid[] = "$Id: terminal.c,v 1.5 1994/07/03 12:31:26 johnsonm Exp $";


void set_terminal(void) {

  static struct termios term;
  static int already=0;

  if (!already) {
    tcgetattr(STDIN_FILENO, &oterm);
    term=oterm;
    term.c_iflag &= !BRKINT;
    term.c_iflag |= IGNBRK;
    term.c_lflag &= !(ECHO | ECHOCTL | ISIG);
    already=1;
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &term);

}



void restore_terminal(void) {

  if (is_vt) {
    ioctl(vfd, VT_SETMODE, &ovtm);
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &oterm);

}


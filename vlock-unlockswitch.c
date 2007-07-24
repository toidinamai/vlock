/* vlock-unlockswitch.c -- console switch unlocking routine for vlock,
 *                         the VT locking program for linux
 *
 * This program is copyright (C) 2007 Frank Benkstein, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/vt.h>

#define CONSOLE "/dev/tty0"

int main(void) {
  int consfd;

  if ((consfd = open(CONSOLE, O_RDWR)) < 0) {
    perror("vlock: cannot open virtual console");
    exit (1);
  }

  /* globally enable virtual console switching */
  if (ioctl(consfd, VT_UNLOCKSWITCH) < 0) {
    perror("vlock-lockswitch: could not disable console switching");
    exit (1);
  }

  return 0;
}

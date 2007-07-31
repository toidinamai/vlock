/* vlock.h -- main header file for vlock, the VT locking program for linux
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

/* name of the virtual console device */
#define CONSOLE "/dev/tty0"
/* template for the device of a given virtual console */
#define VTNAME "/dev/tty%d"

/* magic characters to clear the current terminal */
#define CLEAR_SCREEN "\033[H\033[J"

/* Try to authenticate the user.  When the user is successfully authenticated
 * this function returns 1.  When the authentication fails for whatever reason
 * the function returns 0.
 */
int auth(const char *user);

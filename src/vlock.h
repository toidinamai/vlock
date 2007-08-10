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
#ifdef __FreeBSD__
#define CONSOLE "/dev/ttyv0"
#else
#define CONSOLE "/dev/tty0"
#endif
/* template for the device of a given virtual console */
#ifdef __FreeBSD__
#define VTNAME "/dev/ttyv%x"
#else
#define VTNAME "/dev/tty%d"
#endif

/* magic characters to clear the current terminal */
#define CLEAR_SCREEN "\033[H\033[J"

/* hard coded paths */
#define VLOCK_ALL PREFIX "/sbin/vlock-all"
#define VLOCK_CURRENT PREFIX "/sbin/vlock-current"
#define VLOCK_NEW PREFIX "/sbin/vlock-new"

/* Try to authenticate the user.  When the user is successfully authenticated
 * this function returns 1.  When the authentication fails for whatever reason
 * the function returns 0.
 */
int auth(const char *user);

/* Prompt for a string with the given message.  The string is returned if
 * successfully read, otherwise NULL. */
char *prompt(const char *msg);

/* Prompt for a string with the given message without echoing input characters
 * The string is returned if successfully read, NULL otherwise. */
char *prompt_echo_off(const char *msg);

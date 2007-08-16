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

/* hard coded paths */
#define VLOCK_ALL PREFIX "/sbin/vlock-all"
#define VLOCK_MAIN PREFIX "/sbin/vlock-main"
#define VLOCK_NEW PREFIX "/sbin/vlock-new"
#define VLOCK_PLUGIN_DIR PREFIX "/lib/vlock/plugins"

/* forward declaration */
struct timespec;

/* Try to authenticate the user.  When the user is successfully authenticated
 * this function returns 1.  When the authentication fails for whatever reason
 * the function returns 0.  The timeout is passed to the prompt functions below
 * if they are called.
 */
int auth(const char *user, const struct timespec *timeout);

/* Prompt for a string with the given message.  The string is returned if
 * successfully read, otherwise NULL.  The caller is responsible for freeing
 * the resulting buffer.  If no string is read after the given timeout this
 * prompt() returns NULL.  A timeout of NULL means no timeout, i.e. wait forever.
 */
char *prompt(const char *msg, const struct timespec *timeout);

/* Same as prompt() above, except that characters entered are not echoed. */
char *prompt_echo_off(const char *msg, const struct timespec *timeout);

#ifdef USE_PLUGINS
void load_plugins(void);

void plugin_hook(const char *name);
#endif

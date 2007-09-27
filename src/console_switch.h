/* console_switch.c -- header file for the console grabbing
 *                     routines for vlock, the VT locking program
 *                     for linux
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

#include <stdbool.h>

/* Is console switching currently disabled? */
extern bool console_switch_locked;

/* Disable virtual console switching in the kernel.  If disabling fails false
 * is returned and *error is set to a diagnostic message that must be freed by
 * the caller.  */
bool lock_console_switch(char **error);
/* Reenable console switching if it was previously disabled. */
void unlock_console_switch(void);

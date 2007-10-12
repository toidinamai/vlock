/* vlock_plugin.h -- header file for plugins for vlock,
 *                   the VT locking program for linux
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

/* A plugin might define these arrays to indicate
 * that its must always be called before or after
 * the ones listed in the respective arrays.  If
 * set, the arrays must be terminated by NULL.
 */
extern const char *preceeds[];
extern const char *succeeds[];

/* A plugin might define this array to indicate
 * that the listed plugins should also be loaded
 * when this plugin is invoked.
 */
extern const char *requires[];

/* A plugin might define this array to indicate
 * that it will fail if the named plugins are not
 * also loaded.
 */
extern const char *needs[];

/* A plugin might define this array to indicate
 * that it should be (silently) unloaded if the
 * named plugins are not also loaded.
 */
extern const char *depends[];

/* A plugin might define thes array to indicate
 * that it is not possible to have it loaded at
 * the same time as any of the other plugins
 * listed. */
extern const char *conflicts[];

/* This hook is invoked at the start of vlock.  It is provided with a reference
 * to a pointer to NULL that may be changed to point to an arbitrary context
 * object.
 *
 * On success it should return true and false on error.  If this function
 * fails, vlock will not continue.
 */
bool vlock_start(void **);

/* This hook is invoked when vlock is about to exit.  Its argument is a
 * reference to the same pointer previously given to vlock_start.  
 *
 * On success it should return true and false on error.  If this function
 * fails, vlock will not continue.
 */
bool vlock_end(void **);

/* This hook is invoked whenever a timeout is reached or Escape is pressed
 * after locking the screen.  Rules for the argument are the same as for
 * vlcok_start, except that the pointer may already point to valid data if set
 * in a previous run.
 *
 * On success it should return true and false on error.  If this function
 * fails, vlock will not continue.
 */
bool vlock_save(void **);

/* This hook is invoked to abort the previous one.  The pointer argument is the
 * same as for vlock_save.
 *
 * On success it should return true and false on error.  If this function
 * fails, vlock will not continue.
 */
bool vlock_save_abort(void **);

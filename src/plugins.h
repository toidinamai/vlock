/* plugins.h -- plugins header file for vlock,
 *              the VT locking program for linux
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

#ifdef __cplusplus
extern "C" {
#endif

/* Load the named plugin.  Returns true if loading was successful and false
 * otherwise. */
bool load_plugin(const char *name);

/* Check if the named plugin is loaded. */
bool is_loaded(const char *name);

/* Resolve all the dependencies between all plugins.  Returns false if there
 * was an error and true otherwise.  This function *must* be called after all
 * plugins were loaded. */
bool resolve_dependencies(void);

/* Unload all plugins. */
void unload_plugins(void);

/* Call the given plugin hook. */
bool plugin_hook(const char *hook);

#ifdef __cplusplus
}
#endif

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

/* Load the named plugin from the specified directory.  Returns true if loading
 * was successful and false otherwise. */
bool load_plugin(const char *name, const char *plugin_dir);

/* Resolve all the dependencies between all plugins.  Returns false if there
 * was an error and true otherwise.  This function *must* be called after all
 * plugins were loaded. */
bool resolve_dependencies(void);

/* Unload all plugins. */
void unload_plugins(void);

/* Call the given plugin hook. */
bool plugin_hook(const char *hook);

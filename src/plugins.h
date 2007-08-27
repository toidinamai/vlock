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

int load_plugin(const char *name, const char *plugin_dir);
void unload_plugins(void);
int resolve_dependencies(void);

#define HOOK_VLOCK_START 0
#define HOOK_VLOCK_END 1
#define HOOK_VLOCK_SAVE 2
#define HOOK_VLOCK_SAVE_ABORT 3

#define NR_HOOKS 4

extern const char *hook_names[];

int plugin_hook(unsigned int hook);

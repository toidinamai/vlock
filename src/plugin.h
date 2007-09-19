/* plugin.h -- header file for the generic plugin routines for vlock,
 *             the VT locking program for linux
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

/* Names of dependencies plugins may specify. */
#define nr_dependencies 6
extern const char *dependency_names[nr_dependencies];

/* A plugin hook consists of a name and a handler function. */
struct hook
{
  const char *name;
  void (*handler)(const char *);
};

/* Hooks that a plugin may define. */
#define nr_hooks 4
extern const struct hook hooks[nr_hooks];

/* A single plugin is represented by this struct. */
struct plugin
{
  /* The name of the plugin. */
  char *name;

  /* Array of dependencies.  Each dependency is a (possibly empty) list of
   * strings.  The dependencies must be stored in the same order as the
   * dependency names above.  The strings a freed when the plugin is destroyed
   * thus must be stored as copies. */
  struct list *dependencies[nr_dependencies];

  /* Did one of the save hooks fail? */
  bool save_disabled;

  /* Function that is to be called when a hook should be executed.  */
  bool (*call_hook)(struct plugin *p, const char *name);
  /* Function that is to be called when this plugin should be destroyed.  */
  void (*close)(struct plugin *p);

  /* The call_hook and close functions may use this pointer. */
  void *context;
};

/* Create a new module type plugin.  On failure *error may be set to a
 * diagnostic message that must be freed by the caller and NULL is returned. */
struct plugin *open_module(const char *name, char **error);

/* Create a new script type plugin.  On failure *error may be set to a
 * diagnostic message that must be freed by the caller and NULL is returned. */
struct plugin *open_script(const char *name, char **error);

/* Allocate a new plugin struct.  Only the name field is set to a copy of the
 * given name.  The dependencies array is intialized with empty lists and
 * save_disabled is set to false.  This function should not be called directly.
 */
struct plugin *__allocate_plugin(const char *name);

/* Destroy the given plugin.  This is the opposite of of __allocate_plugin.
 * This function should not be called directly. */
void __destroy_plugin(struct plugin *p);

/* Destroy the given plugin by calling its close function and then
 * __destroy_plugin. */
void destroy_plugin(struct plugin *p);

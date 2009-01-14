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

#pragma once

#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>

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

struct plugin_type;

/* Struct representing a plugin instance. */
struct plugin
{
  /* The name of the plugin. */
  char *name;

  /* Array of dependencies.  Each dependency is a (possibly empty) list of
   * strings.  The dependencies must be stored in the same order as the
   * dependency names above.  The strings a freed when the plugin is destroyed
   * thus must be stored as copies. */
  GList *dependencies[nr_dependencies];

  /* Did one of the save hooks fail? */
  bool save_disabled;

  /* The type of the plugin. */
  struct plugin_type *type;

  /* The call_hook and close functions may use this pointer. */
  void *context;
};


/* Struct representing the type of a plugin. */
struct plugin_type
{
  /* Method that is called on plugin initialization. */
  bool (*init)(struct plugin *p);
  /* Method that is called on plugin destruction.  */
  void (*destroy)(struct plugin *p);
  /* Method that is called when a hook should be executed.  */
  bool (*call_hook)(struct plugin *p, const char *name);
};

/* Modules. */
extern struct plugin_type *module;
/* Scripts. */
extern struct plugin_type *script;

/* Open a new plugin struct of the given type.  On error errno is set and NULL
 * is returned. */ 
struct plugin *new_plugin(const char *name, struct plugin_type *type);

/* Destroy the given plugin.  This is the opposite of of __allocate_plugin.
 * This function should not be called directly. */
void destroy_plugin(struct plugin *p);

/* Call the hook of a plugin. */
bool call_hook(struct plugin *p, const char *hook_name);

/*
 * Type macros
 */
#define TYPE_VLOCK_PLUGIN (vlock_plugin_get_type())
#define VLOCK_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_VLOCK_PLUGIN, VlockPlugin))
#define VLOCK_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_VLOCK_PLUGIN, VlockPluginClass))
#define IS_VLOCK_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_VLOCK_PLUGIN))
#define IS_VLOCK_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_VLOCK_PLUGIN))
#define VLOCK_PLUGIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_VLOCK_PLUGIN, VlockPluginClass))

typedef struct _VlockPlugin VlockPlugin;
typedef struct _VlockPluginClass VlockPluginClass;

struct _VlockPlugin
{
  GObject *parent_instance;
  gchar *name;

  GList *dependencies[nr_dependencies];
};

struct _VlockPluginClass
{
  GObjectClass parent_class;
};

GType vlock_plugin_get_type(void);

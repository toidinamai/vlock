/* plugins.c -- plugin routines for vlock,
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

#ifndef __FreeBSD__
/* for asprintf() */
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <assert.h>

#include "vlock.h"
#include "plugins.h"
#include "tsort.h"
#include "list.h"

#define ARRAY_SIZE(x) ((sizeof (x) / sizeof (x[0])))

/* hard coded paths */
#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

/* dependency names */
const char *dependency_names[] = {
  "after",
  "before",
  "requires",
  "needs",
  "depends",
  "conflicts",
};

/* hook names */
const char *hook_names[] = {
  "vlock_start",
  "vlock_end",
  "vlock_save",
  "vlock_save_abort",
};

static bool handle_vlock_start(int hook_index);
static bool handle_vlock_end(int hook_index);
static bool handle_vlock_save(int hook_index);
static bool handle_vlock_save_abort(int hook_index);

bool (*hook_handlers[])(int) = {
  handle_vlock_start,
  handle_vlock_end,
  handle_vlock_save,
  handle_vlock_save_abort,
};

/* function type for hooks */
typedef bool (*vlock_hook_fn)(void **);

/* vlock plugin */
struct plugin {
  /* name of the plugin */
  char *name;
  /* path to the shared object file */
  char *path;

  /* plugin hook context */
  void *ctx;

  /* dl handle */
  void *dl_handle;

  /* dependencies */
  struct List *dependencies[ARRAY_SIZE(dependency_names)];

  /* plugin hook functions */
  vlock_hook_fn hooks[ARRAY_SIZE(hook_names)];
};

/* the list of plugins */
struct List *plugins = NULL;

static int __get_index(const char *a[], size_t l, const char *s) {
  for (size_t i = 0; i < l; i++)
    if (strcmp(a[i], s) == 0)
      return i;

  return -1;
}

#define get_index(a, s) __get_index(a, ARRAY_SIZE(a), s)

#define get_dependency(p, d) \
  (p->dependencies[get_index(dependency_names, d)])

/* Get the plugin with the given name.  Returns the first plugin
 * with the given name or NULL if none could be found. */
static struct plugin *get_plugin(const char *name)
{
  list_for_each(plugins, item) {
    struct plugin *p = item->data;

    if (strcmp(p->name, name) == 0)
      return p;
  }

  return NULL;
}

/* Check if the given plugin is loaded. */
bool is_loaded(const char *name)
{
  return get_plugin(name) != NULL;
}

/* Open the plugin in file with the given name.  Returns the new plugin or NULL
 * on error. */
static struct plugin *open_module(const char *name)
{
  struct plugin *new;

  /* allocate a new plugin object */
  if ((new = malloc((sizeof *new))) == NULL)
    return NULL;

  new->ctx = NULL;
  new->path = NULL;
  new->name = NULL;

  /* format the plugin path */
  if (asprintf(&new->path, "%s/%s.so", VLOCK_MODULE_DIR, name) < 0)
    goto err;

  /* remember the name */
  if (asprintf(&new->name, "%s", name) < 0)
    goto err;

  /* load the plugin */
  new->dl_handle = dlopen(new->path, RTLD_NOW | RTLD_LOCAL);

  if (new->dl_handle == NULL) {
    fprintf(stderr, "vlock-plugins: %s\n", dlerror());
    goto err;
  }

  /* load the hooks, unimplemented hooks are NULL */
  for (size_t i = 0; i < ARRAY_SIZE(new->hooks); i++)
    *(void **) (&new->hooks[i]) = dlsym(new->dl_handle, hook_names[i]);

  /* load dependencies */
  for (size_t i = 0; i < ARRAY_SIZE(new->dependencies); i++) {
    const char *(*dependency)[] = dlsym(new->dl_handle, dependency_names[i]);
    new->dependencies[i] = NULL;

    for (size_t j = 0; dependency != NULL && (*dependency)[j] != NULL; j++)
      new->dependencies[i]  = list_append(new->dependencies[i], strdup((*dependency)[j]));
  }

  return new;

err:
  free(new->path);
  free(new->name);
  free(new);

  return NULL;
}

/* Same as open_plugin except that an old plugin is returned if there
 * already is one with the given name. */
static struct plugin *__load_plugin(const char *name)
{
  struct plugin *p = get_plugin(name);

  if (p != NULL)
    return p;

  p = open_module(name);

  plugins = list_append(plugins, p);

  return p;
}

/* Same as __load_plugin except that true is returned on success and false on
 * error. */
bool load_plugin(const char *name)
{
  return __load_plugin(name) != NULL;
}

/* Unload the given plugin and remove from the plugins list. */
static void unload_plugin(struct List *item)
{
  struct plugin *p = item->data;
  plugins = list_delete_link(plugins, item);
  (void) dlclose(p->dl_handle);
  free(p->path);
  free(p->name);

  for (size_t i = 0; i < ARRAY_SIZE(p->dependencies); i++)
    list_for_each(p->dependencies[i], dependency_item)
      free(dependency_item->data);

  free(p);
}

/* Unload all plugins */
void unload_plugins(void)
{
  while (plugins != NULL)
    unload_plugin(plugins);
}

/* Forward declaration */
static bool sort_plugins(void);

bool resolve_dependencies(void)
{
  struct List *required_plugins = NULL;

  /* load plugins that are required, this automagically takes care of plugins
   * that are required by the plugins loaded here because they are appended to
   * the end of the list */
  list_for_each(plugins, item) {
    struct plugin *p = item->data;

    list_for_each(get_dependency(p, "requires"), dependency_item) {
      struct plugin *d = __load_plugin(dependency_item->data);

      if (d == NULL)
        goto err;

      required_plugins = list_append(required_plugins, d);
    }
  }

  /* fail if a plugins that is needed is not loaded */
  list_for_each(plugins, item) {
    struct plugin *p = item->data;

    list_for_each(get_dependency(p, "needs"), dependency_item) {
      char *dependency_name = dependency_item->data;
      struct plugin *d = get_plugin(dependency_name);

      if (d == NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n",
                p->name, dependency_name);
        goto err;
      }

      required_plugins = list_append(required_plugins, d);
    }
  }

  /* unload plugins whose prerequisites are not present */
  for (struct List *item = list_first(plugins); item != NULL;) {
    struct plugin *p = item->data;
    struct List *tmp = item;
    item = list_next(item);

    list_for_each(get_dependency(p, "depends"), dependency_item) {
      char *dependency_name = dependency_item->data;
      struct plugin *d = get_plugin(dependency_name);

      if (d != NULL)
        continue;

      if (list_find(required_plugins, p) != NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n",
                p->name, dependency_name);
        goto err;
      } else {
        unload_plugin(tmp);
        break;
      }
    }
  }

  list_free(required_plugins);

  /* fail if conflicting plugins are loaded */
  list_for_each(plugins, item) {
    struct plugin *p = item->data;
    list_for_each(get_dependency(p, "conflicts"), dependency_item) {
      char *dependency_name = dependency_item->data;

      if (get_plugin(dependency_name) != NULL) {
        fprintf(stderr,
                "vlock-plugins: %s and %s cannot be loaded at the same time\n",
                p->name, dependency_name);
        return false;
      }
    }
  }

  return sort_plugins();

err:
  list_free(required_plugins);
  return false;
}

/* A plugin may declare which plugins it must come before or after.  All those
 * relations together form a directed (acyclic) graph.  This graph is sorted
 * using a topological sort.
 */

/* Get all edges as specified by the all plugins' "after" and "before"
 * attributes. */
static struct List *get_edges(void)
{
  struct List *edges = NULL;

  list_for_each(plugins, item) {
    struct plugin *p = item->data;
    /* p must come after these */
    struct List *predecessors = get_dependency(p, "after");
    /* p must come before these */
    struct List *successors = get_dependency(p, "before");

    list_for_each(successors, item) {
      struct plugin *successor = get_plugin(item->data);

      if (successor != NULL) {
        struct Edge *edge = malloc(sizeof *edge);
        edge->predecessor = p;
        edge->successor = successor;
        edges = list_append(edges, edge);
      }
    }

    list_for_each(predecessors, item) {
      struct plugin *predecessor = get_plugin(item->data);

      if (predecessor != NULL) {
        struct Edge *edge = malloc(sizeof *edge);
        edge->predecessor = predecessor;
        edge->successor = p;
        edges = list_append(edges, edge);
      }
    }
  }

  return edges;
}

static bool sort_plugins(void)
{
  struct List *edges = get_edges();
  struct List *sorted_plugins = tsort(plugins, &edges);

  if (edges == NULL) {
    struct List *tmp = plugins;
    plugins = sorted_plugins;
    list_free(tmp);

    return true;
  } else {
    fprintf(stderr, "vlock-plugins: circular dependencies detected:\n");

    list_for_each(edges, item) {
      struct Edge *edge = item->data;
      struct plugin *p = edge->predecessor;
      struct plugin *s = edge->successor;
      fprintf(stderr, "\t%s\tmust come before\t%s\n", p->name, s->name);
      free(edge);
    }

    list_free(edges);

    return false;
  }
}

/* Call the given plugin hook. */
bool plugin_hook(const char *hook_name)
{
  int hook_index = get_index(hook_names, hook_name);

  if (hook_index < 0) {
    fprintf(stderr, "vlock-plugins: unknown hook '%s'\n", hook_name);
    return false;
  } else {
    return hook_handlers[hook_index] (hook_index);
  }
}

static bool handle_vlock_start(int hook_index)
{
  list_for_each(plugins, item) {
    struct plugin *p = item->data;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL)
      if (!hook(&p->ctx))
        return false;
  }

  return true;
}

static bool handle_vlock_end(int hook_index)
{
  list_reverse_for_each(plugins, item) {
    struct plugin *p = item->data;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL)
      (void) hook(&p->ctx);
  }

  return true;
}

/* Return true if at least one hook was called and all hooks were successful.
 * Does not continue after the first failing hook. */
static bool handle_vlock_save(int hook_index)
{
  bool result = false;

  list_for_each(plugins, item) {
    struct plugin *p = item->data;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL) {
      result = hook(&p->ctx);

      if (!result) {
        /* don't call again */
        p->hooks[hook_index] = NULL;
        result = false;
        break;
      }
    }
  }

  return result;
}

static bool handle_vlock_save_abort(int hook_index)
{
  list_reverse_for_each(plugins, item) {
    struct plugin *p = item->data;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL) {
      int vlock_save_index = get_index(hook_names, "vlock_save");

      if (!hook(&p->ctx) || p->hooks[vlock_save_index] == NULL) {
        /* don't call again */
        p->hooks[hook_index] = NULL;
        p->hooks[vlock_save_index] = NULL;
      }
    }
  }

  return true;
}

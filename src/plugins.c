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

/* hook names */
const char *hook_names[] = {
  "vlock_start",
  "vlock_end",
  "vlock_save",
  "vlock_save_abort",
};

enum hooks {
  HOOK_VLOCK_START = 0,
  HOOK_VLOCK_END,
  HOOK_VLOCK_SAVE,
  HOOK_VLOCK_SAVE_ABORT,
};

const size_t nr_hooks = (sizeof (hook_names)/sizeof (hook_names[0]));

int get_hook_index(const char *name) {
  for (size_t i = 0; i < nr_hooks; i++)
    if (strcmp(hook_names[i], name) == 0)
      return i;

  return -1;
}

/* function type for hooks */
typedef int (*vlock_hook_fn)(void **);

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

  /* plugin hook functions */
  vlock_hook_fn hooks[];
};

/* the list of plugins */
struct List *plugins = NULL;

/* Get the plugin with the given name.  Returns the first plugin
 * with the given name or NULL if none could be found. */
static struct plugin *get_plugin(const char *name) {
  for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
    struct plugin *p = item->data;

    if (strcmp(p->name, name) == 0)
      return p;
  }

  return NULL;
}

/* Open the plugin in file with the given name at the specified location.
 * Returns the new plugin or NULL on error. */
static struct plugin *open_plugin(const char *name, const char *plugin_dir) {
  struct plugin *new;

  /* allocate a new plugin object */
  if ((new = malloc((sizeof *new) + nr_hooks * (sizeof (vlock_hook_fn)))) == NULL)
    return NULL;

  new->ctx = NULL;
  new->path = NULL;
  new->name = NULL;

  /* format the plugin path */
  if (asprintf(&new->path, "%s/%s.so", plugin_dir, name) < 0)
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
  for (size_t i = 0; i < nr_hooks; i++) {
    *(void **) (&new->hooks[i]) = dlsym(new->dl_handle, hook_names[i]);
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
static struct plugin *__load_plugin(const char *name, const char *plugin_dir) {
  struct plugin *p = get_plugin(name);

  if (p != NULL)
    return p;

  p = open_plugin(name, plugin_dir);
    plugins = list_append(plugins, p);

  return p;
}

/* Same as __load_plugin except that true is returned on success and false on
 * error. */
bool load_plugin(const char *name, const char *plugin_dir) {
  return __load_plugin(name, plugin_dir) != NULL;
}

/* Unload the given plugin and remove from the plugins list. */
static void unload_plugin(struct plugin *p) {
  plugins = list_remove(plugins, p);
  (void) dlclose(p->dl_handle);
  free(p->path);
  free(p->name);
  free(p);
}

/* Unload all plugins */
void unload_plugins(void) {
  for (struct List *item = list_first(plugins); item != NULL; item = list_first(plugins))
    unload_plugin(item->data);
}

/* Forward declaration */
static bool sort_plugins(void);

bool resolve_dependencies(void) {
  struct List *required_plugins = NULL;

  /* load plugins that are required */
  for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
    struct plugin *p = item->data;
    const char *(*requires)[] = dlsym(p->dl_handle, "requires");

    for (int i = 0; requires != NULL && (*requires)[i] != NULL; i++) {
      struct plugin *d = __load_plugin((*requires)[i], VLOCK_PLUGIN_DIR);

      if (d == NULL)
        goto err;

      required_plugins = list_append(required_plugins, d);
    }
  }

  /* fail if a plugins that is needed is not loaded */
  for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
    struct plugin *p = item->data;
    const char *(*needs)[] = dlsym(p->dl_handle, "needs");

    for (int i = 0; needs != NULL && (*needs)[i] != NULL; i++) {
      struct plugin *d = get_plugin((*needs)[i]); 

      if (d == NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n", p->name, (*needs)[i]);
        goto err;
      }

      required_plugins = list_append(required_plugins, d);
    }
  }

  /* unload plugins whose prerequisites are not present */
  for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
    struct plugin *p = item->data;
    const char *(*depends)[] = dlsym(p->dl_handle, "depends");

    for (int i = 0; depends != NULL && (*depends)[i] != NULL; i++) {
      struct plugin *d = get_plugin((*depends)[i]);

      if (d != NULL)
        continue;

      if (list_find(required_plugins, p) != NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n", p->name, (*depends)[i]);
        goto err;
      } else {
        unload_plugin(p); 
        break;
      }
    }
  }

  list_free(required_plugins);

  /* fail if conflicting plugins are loaded */
  for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
    struct plugin *p = item->data;
    const char *(*conflicts)[] = dlsym(p->dl_handle, "conflicts");

    for (int i = 0; conflicts != NULL && (*conflicts)[i] != NULL; i++) {
      if (get_plugin((*conflicts)[i]) != NULL) {
        fprintf(stderr, "vlock-plugins: %s and %s cannot be loaded at the same time\n", p->name, (*conflicts)[i]);
        return -1;
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
static struct List *get_edges(void) {
  struct List *edges = NULL;

  for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
    struct plugin *p = item->data;
    /* p must come after these */
    const char *(*predecessors)[] = dlsym(p->dl_handle, "after");
    /* p must come before these */
    const char *(*successors)[] = dlsym(p->dl_handle, "before");

    for (int i = 0; successors != NULL && (*successors)[i] != NULL; i++) {
      struct plugin *successor = get_plugin((*successors)[i]);

      if (successor != NULL) {
        struct Edge *edge = malloc(sizeof *edge);
        edge->predecessor = p;
        edge->successor = successor;
        edges = list_append(edges, edge);
      }
    }

    for (int i = 0; predecessors != NULL && (*predecessors)[i] != NULL; i++) {
      struct plugin *predecessor = get_plugin((*predecessors)[i]);

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

static bool sort_plugins(void) {
  struct List *edges = get_edges();
  struct List *sorted_plugins = tsort(plugins, &edges);

  if (edges == NULL) {
    struct List *tmp = plugins;
    plugins = sorted_plugins;
    list_free(tmp);

    return true;
  } else {
    fprintf(stderr, "vlock-plugins: circular dependencies detected:\n");

    for (struct List *item = list_first(edges); item != NULL; item = list_next(item)) {
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
int plugin_hook(const char *hook_name) {
  int hook_index = get_hook_index(hook_name);

  if (hook_index == HOOK_VLOCK_START || hook_index == HOOK_VLOCK_SAVE) {
    for (struct List *item = list_first(plugins); item != NULL; item = list_next(item)) {
      struct plugin *p = item->data;
      vlock_hook_fn hook = p->hooks[hook_index];
      int result;

      if (hook == NULL)
        continue;

      result = hook(&p->ctx);

      if (result != 0) {
        if (hook_index == HOOK_VLOCK_START)
          return result;
        else if (hook_index == HOOK_VLOCK_SAVE)
          /* don't call again */
          p->hooks[hook_index] = NULL;
      }
    }
  } else if (hook_index == HOOK_VLOCK_END || hook_index == HOOK_VLOCK_SAVE_ABORT) {
    for (struct List *item = list_last(plugins); item != NULL; item = list_previous(item)) {
      struct plugin *p = item->data;
      vlock_hook_fn hook = p->hooks[hook_index];
      int result;

      if (hook == NULL)
        continue;

      result = hook(&p->ctx);

      if (result != 0 && hook_index == HOOK_VLOCK_SAVE_ABORT) {
        /* don't call again */
        p->hooks[hook_index] = NULL;
        p->hooks[HOOK_VLOCK_SAVE] = NULL;
      }
    }
  } else {
    fprintf(stderr, "vlock-plugins: unknown hook '%s'\n", hook_name);
    return -1;
  }

  return 0;
}

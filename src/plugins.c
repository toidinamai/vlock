#ifndef __FreeBSD__
/* for asprintf() */
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <glib.h>

#include "vlock.h"
#include "plugins.h"

/* hook names */
const char const *hook_names[] = {
  "vlock_start",
  "vlock_end",
  "vlock_save",
  "vlock_save_abort",
};

/* function type for hooks */
typedef int (*vlock_hook_fn)(void **);

/* vlock plugin */
struct plugin {
  /* name of the plugin */
  char *name;
  /* path to the shared object file */
  char *path;

  /* plugin hook functions */
  vlock_hook_fn hooks[NR_HOOKS];

  /* plugin hook context */
  void *ctx;

  /* dl handle */
  void *dl_handle;
};

GList *plugins = NULL;

static gint compare_name(gconstpointer p, gconstpointer n) {
  return strcmp(((struct plugin *)p)->name, (const char *)n);
}

static struct plugin *get_plugin(const char *name) {
  GList *item = g_list_find_custom(plugins, name, &compare_name);

  if (item != NULL)
    return item->data;
  else
    return NULL;
}

static struct plugin *open_plugin(const char *name, const char *plugin_dir) {
  struct plugin *new;
  int i;

  /* allocate a new plugin object */
  if ((new = malloc(sizeof *new)) == NULL)
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
    fprintf(stderr, "%s\n", dlerror());
    goto err;
  }

  /* load the hooks, unimplemented hooks are NULL */
  for (i = 0; i < NR_HOOKS; i++) {
    *(void **) (&new->hooks[i]) = dlsym(new->dl_handle, hook_names[i]);
  }

  return new;

err:
  free(new->path);
  free(new->name);
  free(new);

  return NULL;
}

static struct plugin *__load_plugin(const char *name, const char *plugin_dir) {
  struct plugin *p = get_plugin(name);

  if (p != NULL)
    return p;

  p = open_plugin(name, plugin_dir);
    plugins = g_list_append(plugins, p);

  return p;
}

int load_plugin(const char *name, const char *plugin_dir) {
  return - (__load_plugin(name, plugin_dir) == NULL);
}

static void unload_plugin(struct plugin *p) {
  plugins = g_list_remove(plugins, p);
  (void) dlclose(p->dl_handle);
  free(p->path);
  free(p->name);
  free(p);
}

void unload_plugins(void) {
  while (plugins != NULL)
    unload_plugin(plugins->data);
}

static int sort_plugins(void);

int resolve_dependencies(void) {
  GList *required_plugins = NULL;

  /* load plugins that are required */
  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;
    const char *(*requires)[] = dlsym(p->dl_handle, "requires");

    for (int i = 0; requires != NULL && (*requires)[i] != NULL; i++) {
      struct plugin *d = __load_plugin((*requires)[i], VLOCK_PLUGIN_DIR);

      if (d == NULL)
        goto err;

      required_plugins = g_list_append(required_plugins, d);
    }
  }

  /* fail if a plugins that is needed is not loaded */
  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;
    const char *(*needs)[] = dlsym(p->dl_handle, "needs");

    for (int i = 0; needs != NULL && (*needs)[i] != NULL; i++) {
      if (get_plugin((*needs)[i]) == NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n", p->name, (*needs)[i]);
        goto err;
      }
    }
  }

  /* unload plugins whose prerequisites are not present */
  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;
    const char *(*depends)[] = dlsym(p->dl_handle, "depends");

    for (int i = 0; depends != NULL && (*depends)[i] != NULL; i++) {
      struct plugin *d = get_plugin((*depends)[i]);

      if (d != NULL)
        continue;

      if (g_list_find(required_plugins, p) != NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n", p->name, (*depends)[i]);
        goto err;
      }
      else {
        unload_plugin(p); 
        break;
      }
    }
  }

  g_list_free(required_plugins);

  /* fail if conflicting plugins are loaded */
  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;
    const char *(*conflicts)[] = dlsym(p->dl_handle, "conflicts");

    for (int i = 0; conflicts != NULL && (*conflicts)[i] != NULL; i++) {
      if (get_plugin((*conflicts)[i]) != NULL) {
        fprintf(stderr, "vlock-plugins: %s and %s cannot be loaded at the same time\n", p->name, (*conflicts)[i]);
      }
    }
  }

  return sort_plugins();

err:
  g_list_free(required_plugins);
  return -1;
}

struct edge {
  struct plugin *predecessor;
  struct plugin *successor;
};

static GList *get_edges(void) {
  GList *edges = NULL;

  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;
    const char *(*successors)[] = dlsym(p->dl_handle, "after");
    const char *(*predecessors)[] = dlsym(p->dl_handle, "before");

    for (int i = 0; successors != NULL && (*successors)[i] != NULL; i++) {
      struct plugin *successor = get_plugin((*successors)[i]);

      if (successor != NULL) {
        struct edge *edge = malloc(sizeof (struct edge));
        edge->predecessor = p;
        edge->successor = successor;
        edges = g_list_append(edges, edge);
      }
    }

    for (int i = 0; predecessors != NULL && (*predecessors)[i] != NULL; i++) {
      struct plugin *predecessor = get_plugin((*predecessors)[i]);

      if (predecessor != NULL) {
        struct edge *edge = malloc(sizeof (struct edge));
        edge->predecessor = predecessor;
        edge->successor = p;
        edges = g_list_append(edges, edge);
      }
    }
  }

  return edges;
}

static GList *get_zeros(GList *edges) {
  GList *zeros = g_list_copy(plugins);

  for (GList *item = g_list_first(edges); item != NULL && g_list_first(zeros) != NULL; item = g_list_next(item)) {
    struct edge *edge = item->data;

    zeros = g_list_remove(zeros, edge->successor);
  }

  return zeros;
}

static int is_zero(struct plugin *p, GList *edges) {
  for (GList *item = g_list_first(edges); item != NULL; item = g_list_next(item)) {
    struct edge *edge = item->data;
    if (edge->successor == p)
      return 0;
  }

  return 1;
}

static int sort_plugins(void) {
  GList *edges = get_edges();
  GList *zeros = get_zeros(edges);
  GList *sorted_plugins = NULL;

  for (GList *item = g_list_first(zeros); item != NULL; item = g_list_first(zeros)) {
    struct plugin *p = item->data;

    sorted_plugins = g_list_append(sorted_plugins, p);
    zeros = g_list_remove(zeros, p);

    for (GList *item = g_list_first(edges); item != NULL; item = g_list_next(item)) {
      struct edge *edge = item->data;

      if (edge->predecessor != p)
        continue;

      edges = g_list_remove(edges, edge);

      if (is_zero(edge->successor, edges))
        zeros = g_list_append(zeros, edge->successor);
    }
  }

  for (GList *item = g_list_first(sorted_plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;
    fprintf(stderr, "%s\n", p->name);
    return -1;
  }

  if (g_list_length(edges) == 0) {
    GList *tmp = plugins;
    plugins = sorted_plugins;
    g_list_free(tmp);

    return 0;
  } else {
    fprintf(stderr, "vlock-plugins: circular dependencies in plugins detected:\n");

    for (GList *item = g_list_first(edges); item != NULL; item = g_list_next(item)) {
      struct edge *edge = item->data;
      fprintf(stderr, "\t%s\tmust come before\t%s\n", edge->predecessor->name, edge->successor->name);
    }

    g_list_free(sorted_plugins);
    g_list_free(edges);
    g_list_free(zeros);

    return -1;
  }
}

int plugin_hook(unsigned int hook) {
  int result;

  if (hook == HOOK_VLOCK_START || hook == HOOK_VLOCK_SAVE) {
    for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
      struct plugin *p = item->data;

      if (p->hooks[hook] == NULL)
        continue;

      result = p->hooks[hook](&p->ctx);

      if (result != 0) {
        if (hook == HOOK_VLOCK_START)
          return result;
        else if (hook == HOOK_VLOCK_SAVE)
          /* don't call again */
          p->hooks[hook] = NULL;
      }
    }
  }
  else if (hook == HOOK_VLOCK_END || hook == HOOK_VLOCK_SAVE_ABORT) {
    for (GList *item = g_list_last(plugins); item != NULL; item = g_list_next(item)) {
      struct plugin *p = item->data;
      if (p->hooks[hook] == NULL)
        continue;

      result = p->hooks[hook](&p->ctx);

      if (result != 0 && hook == HOOK_VLOCK_SAVE_ABORT) {
        /* don't call again */
        p->hooks[hook] = NULL;
        p->hooks[HOOK_VLOCK_SAVE] = NULL;
      }
    }
  }
  else {
    fprintf(stderr, "vlock-plugins: unknown hook '%d'\n", hook);
    return -1;
  }

  return 0;
}

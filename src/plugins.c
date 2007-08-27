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

static struct plugin *get_plugin(const char *name) {
  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;

    if (strcmp(name, p->name) == 0)
      return p;
  }

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

int load_plugin(const char *name, const char *plugin_dir) {
  if (get_plugin(name) != NULL) {
    return 0;
  } else {
    struct plugin *p = open_plugin(name, plugin_dir);

    if (p == NULL)
      return -1;

    plugins = g_list_append(plugins, p);
    return 0;
  }
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
  return sort_plugins();
}

static int sort_plugins(void) {
  fprintf(stderr, "vlock-plugins: resolving dependencies is not fully implemented\n");
  return -1;
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

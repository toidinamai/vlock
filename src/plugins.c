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

#define BEFORE 0
#define AFTER 1
#define REQUIRES 2
#define NEEDS 3
#define DEPENDS 4
#define CONFLICTS 5

#define NR_DEPENDENCIES 6

/* dependency names */
const char const *dependency_names[] = {
  "before",
  "after",
  "requires",
  "needs",
  "depends",
  "conflicts",
};

/* function type for hooks */
typedef int (*vlock_hook_fn)(void **);

/* vlock plugin */
struct plugin {
  /* name of the plugin */
  char *name;
  /* path to the shared object file */
  char *path;

  /* dependencies */
  const char *(*deps[NR_DEPENDENCIES])[];

  /* is this plugin required by an other plugin? */
  int required;

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

static struct plugin *open_plugin(const char *name, const char *plugin_dir, int required) {
  struct plugin *new;
  int i;

  /* allocate a new plugin object */
  if ((new = malloc(sizeof *new)) == NULL)
    return NULL;

  new->ctx = NULL;
  new->path = NULL;
  new->name = NULL;
  new->required = required;

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

  /* load dependencies */
  for (i = 0; i < NR_DEPENDENCIES; i++) {
    new->deps[i] = dlsym(new->dl_handle, dependency_names[i]);
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
    struct plugin *p = open_plugin(name, plugin_dir, 0);

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

void disable_plugin(struct plugin *p) {
  int i;

  for (i = 0; i < NR_HOOKS; i++) {
    p->hooks[i] = NULL;
  }
}

void unload_plugins(void) {
  while (plugins != NULL)
    unload_plugin(plugins->data);
}

static int sort_plugins(void);

int resolve_dependencies(void) {
  int i;
  GList *unusable_plugins = NULL;

  for (GList *item = g_list_first(plugins); item != NULL; item = g_list_next(item)) {
    struct plugin *p = item->data;

    /* load plugins that are required */
    for (i = 0; p->deps[REQUIRES] != NULL && (*p->deps[REQUIRES])[i] != NULL; i++) {
      struct plugin *d = open_plugin((*p->deps[REQUIRES])[i], VLOCK_PLUGIN_DIR, 1);

      if (d == NULL)
        return -1;

      plugins = g_list_append(plugins, d);
    }

    /* fail if a plugins that is needed is not loaded */
    for (i = 0; p->deps[NEEDS] != NULL && (*p->deps[NEEDS])[i] != NULL; i++) {
      if (get_plugin((*p->deps[NEEDS])[i]) == NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s", p->name, (*p->deps[NEEDS])[i]);
        return -1;
      }
    }

    /* unload plugins whose dependencies are not loaded */
    for (i = 0; p->deps[DEPENDS] != NULL && (*p->deps[DEPENDS])[i] != NULL; i++) {
      if (get_plugin((*p->deps[DEPENDS])[i]) == NULL) {
        if (p->required) {
          fprintf(stderr, "vlock-plugins: %s does not work without %s", p->name, (*p->deps[DEPENDS])[i]);
          return -1;
        }

        unusable_plugins = g_list_append(unusable_plugins, p);
      }
    }

    for (i = 0; p->deps[CONFLICTS] != NULL && (*p->deps[CONFLICTS])[i] != NULL; i++) {
      if (get_plugin((*p->deps[CONFLICTS])[i]) != NULL) {
        fprintf(stderr, "vlock-plugins: %s and %s cannot be loaded at the same time\n", p->name, (*p->deps[CONFLICTS])[i]);
        return -1;
      }
    }
  }

  for (GList *item = g_list_first(unusable_plugins); item != NULL; item = g_list_next(item))
    unload_plugin(item->data);

  g_list_free(unusable_plugins);

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

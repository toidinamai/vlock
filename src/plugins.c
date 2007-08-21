#ifndef __FreeBSD__
/* for asprintf() */
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include "plugins.h"

/* hook names */
const char const *hook_names[NR_HOOKS] = {
  "vlock_start",
  "vlock_end",
  "vlock_save",
  "vlock_save_abort",
};

/* function type for hooks */
typedef int (*vlock_hook_fn)(void **);

/* vlock plugin */
struct plugin {
  char *path;
  char *name;
  char **before;
  char **after;
  vlock_hook_fn hooks[NR_HOOKS];
  void *dl_handle;
  void *ctx;
  struct plugin *next;
  struct plugin *previous;
};

static struct plugin *first = NULL;
static struct plugin *last = NULL;

int load_plugin(const char *name, const char *plugin_dir) {
  struct plugin *new;

  /* allocate a new plugin object */
  if ((new = malloc(sizeof *new)) == NULL)
    return -1;

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
  {
    int i;

    for (i = 0; i < NR_HOOKS; i++) {
      *(void **) (&new->hooks[i]) = dlsym(new->dl_handle, hook_names[i]);
    }
  }

  /* load dependencies */
  new->before = dlsym(new->dl_handle, "before");
  new->after = dlsym(new->dl_handle, "after");

  /* add this plugin to the list */
  if (first == NULL) {
    first = last = new;
    new->previous = NULL;
    new->next = NULL;
  } else {
    last->next = new;
    new->previous = last;
    new->next = NULL;
    last = new;
  }

  return 0;

err:
  free(new->path);
  free(new->name);
  free(new);

  return -1;
}

int resolve_dependencies(void) {
  fprintf(stderr, "vlock-plugins: resolving dependencies is not implemented\n");
  return -1;
}

void unload_plugins(void) {
  while (first != NULL) {
    void *tmp = first->next;
    (void) dlclose(first->dl_handle);
    free(first->path);
    free(first->name);
    free(first);
    first = tmp;
  }
}

int plugin_hook(unsigned int hook) {
  struct plugin *current;
  int result;

  if (hook == HOOK_VLOCK_START || hook == HOOK_VLOCK_SAVE) {
    for (current = first; current != NULL; current = current->next) {
      if (current->hooks[hook] == NULL)
        continue;

      result = current->hooks[hook](&current->ctx);

      if (result != 0) {
        if (hook == HOOK_VLOCK_START)
          return result;
        else if (hook == HOOK_VLOCK_SAVE)
          /* don't call again */
          current->hooks[hook] = NULL;
      }
    }
  }
  else if (hook == HOOK_VLOCK_END || hook == HOOK_VLOCK_SAVE_ABORT) {
    for (current = last; current != NULL; current = current->previous) {
      if (current->hooks[hook] == NULL)
        continue;

      result = current->hooks[hook](&current->ctx);

      if (result != 0 && hook == HOOK_VLOCK_SAVE_ABORT) {
        /* don't call again */
        current->hooks[hook] = NULL;
        current->hooks[HOOK_VLOCK_SAVE] = NULL;
      }
    }
  }
  else {
    fprintf(stderr, "vlock-plugins: unknown hook '%d'\n", hook);
    return -1;
  }

  return 0;
}

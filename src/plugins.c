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
  vlock_hook_fn hooks[NR_HOOKS];
  void *dl_handle;
  void *ctx;
  struct plugin *previous;
};

static struct plugin *first = NULL;

int load_plugin(const char *name, const char *plugin_dir) {
  char *plugin_path;
  struct plugin *next;

  /* allocate a new plugin object */
  if ((next = malloc(sizeof *first)) == NULL)
    return -1;

  next->ctx = NULL;

  /* format the plugin path */
  if (asprintf(&plugin_path, "%s/%s.so", plugin_dir, name) < 0)
    goto err;

  /* load the plugin */
  next->dl_handle = dlopen(plugin_path, RTLD_NOW | RTLD_LOCAL);

  free(plugin_path);

  if (next->dl_handle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    goto err;
  }

  /* load the hooks, unimplemented hooks are NULL */
  {
    int i;

    for (i = 0; i < NR_HOOKS; i++) {
      *(void **) (&next->hooks[i]) = dlsym(next->dl_handle, hook_names[i]);
    }
  }

  /* make this plugin first in the list */
  next->previous = first;
  first = next;

  return 0;

err:
  free(next);

  return -1;
}

void unload_plugins(void) {
  while (first != NULL) {
    void *tmp = first->previous;
    (void) dlclose(first->dl_handle);
    free(first);
    first = tmp;
  }
}

int plugin_hook(enum plugin_hook_t hook) {
  struct plugin *current;

  for (current = first; current != NULL; current = current->previous) {
    int result;

    if (hook > MAX_HOOK) {
      fprintf(stderr, "vlock-plugins: unknown hook '%d'\n", hook);
      return -1;
    }

    if (current->hooks[hook] == NULL)
      continue;

    result = current->hooks[hook](&current->ctx);

    if (result != 0) {
      switch (hook) {
        case HOOK_VLOCK_START:
          return result;
        case HOOK_VLOCK_SAVE_ABORT:
          current->hooks[HOOK_VLOCK_SAVE] = NULL;
        case HOOK_VLOCK_SAVE:
          /* don't call again */
          current->hooks[hook] = NULL;
          break;
        default:
          /* ignore */
          break;
      }
    }
  }

  return 0;
}

#include <stdio.h>

#include "plugins.h"

/* vlock plugin */
struct plugin {
  vlock_start_t vlock_start;
  vlock_end_t vlock_end;
  vlock_save_t vlock_save;
  vlock_save_abort_t vlock_save_abort;
  void *dl_handle;
  struct plugin *next;
};

int load_plugin(const char *name) {
  fprintf(stderr, "loading plugin '%s'\n", name);
  return -1;
}

void unload_plugins(void) {
}

int plugin_hook(const char *name) {
  fprintf(stderr, "executing plugin hook '%s'\n", name);
  return -1;
}

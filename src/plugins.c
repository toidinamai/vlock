#include <stdio.h>

#include "plugins.h"

/* vlock plugin */
struct plugin {
  void (*vlock_start)(void **ctx_ptr);
  void (*vlock_end)(void *ctx);
  void (*vlock_save)(void **ctx_ptr);
  void (*vlock_save_abort)(void *ctx);
  void *dl_handle;
  struct plugin *next;
};

int load_plugin(const char *name) {
  fprintf(stderr, "loading plugin '%s'\n", name);
  return 0;
}

int plugin_hook(const char *name) {
  fprintf(stderr, "executing plugin hook '%s'\n", name);
  return 0;
}

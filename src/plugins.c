#include <stdio.h>

#include "vlock.h"

/* vlock plugin */
struct plugin {
  void (*vlock_start)(void **ctx_ptr);
  void (*vlock_end)(void *ctx);
  void (*vlock_save)(void **ctx_ptr);
  void (*vlock_save_abort)(void *ctx);
  void *dl_handle;
  struct plugin *next;
};

void load_plugins(void) {
  fprintf(stderr, "loading plugins\n");
}

void plugin_hook(const char *name) {
  fprintf(stderr, "executing plugin hook '%s'\n", name);
}

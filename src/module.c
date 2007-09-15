#if !defined(__FreeBSD__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/types.h>

#include "plugin.h"
#include "util.h"

#include "module.h"

#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

struct module_context
{
  void *dl_handle;
};

struct plugin *open_module(const char *name, char **error)
{
  char *path;
  struct plugin *m = __allocate_plugin(name);
  struct module_context *context = ensure_malloc(sizeof (struct module_context));

  if (asprintf(&path, "%s/%s.so", VLOCK_MODULE_DIR, name) < 0) {
    *error = strdup("filename too long");
    free(context);
    __destroy_plugin(m);
    return NULL;
  }

  if (access(path, R_OK) < 0) {
    (void) asprintf(error, "%s: %s", path, strerror(errno));
    free(path);
    free(context);
    __destroy_plugin(m);
    return NULL;
  }

  context->dl_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (context->dl_handle == NULL) {
    *error = strdup(dlerror());
    free(path);
    free(context);
    __destroy_plugin(m);
    return NULL;
  }

  free(path);

  m->context = context;

  return m;
}

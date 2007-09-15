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

#include "list.h"
#include "util.h"

#include "plugin.h"

#include "module.h"

#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

typedef bool (*vlock_hook_fn)(void **);
typedef vlock_hook_fn (*hook_dlsym_t)(void *, const char *);

struct module_context
{
  void *dl_handle;
  vlock_hook_fn *hooks;
};

static void close_module(struct plugin *m);

struct plugin *open_module(const char *name, char **error)
{
  char *path;
  struct plugin *m = __allocate_plugin(name);
  struct module_context *context = ensure_malloc(sizeof (struct module_context));

  if (asprintf(&path, "%s/%s.so", VLOCK_MODULE_DIR, name) < 0) {
    *error = strdup("filename too long");
    goto path_error;
  }

  if (access(path, R_OK) < 0) {
    (void) asprintf(error, "%s: %s", path, strerror(errno));
    goto file_error;
  }

  context->dl_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (context->dl_handle == NULL) {
    *error = strdup(dlerror());
    goto file_error;
  }

  free(path);

  context->hooks = ensure_malloc(list_length(hook_names) * sizeof (vlock_hook_fn));

  size_t i = 0;

  list_for_each(hook_names, name_item)
    *(void **) (&context->hooks[i++]) = dlsym(context->dl_handle, name_item->data);

  m->context = context;
  m->close = close_module;

  return m;

file_error:
  free(path);

path_error:
  free(context);
  __destroy_plugin(m);
  return NULL;
}

static void close_module(struct plugin *m)
{
  struct module_context *context = m->context;
  dlclose(context->dl_handle);
}

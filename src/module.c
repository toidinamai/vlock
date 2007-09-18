/* module.c -- module routines for vlock, the VT locking program for linux
 *
 * This program is copyright (C) 2007 Frank Benkstein, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

/* Modules are shared objects that are loaded into vlock's address space. */
/* They can define certain functions that are called through vlock's plugin
 * mechanism.  They should also define dependencies if they depend on other
 * plugins of have to be called before or after other plugins. */

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

/* A hook function as defined by a module. */
typedef bool (*module_hook_function)(void **);

struct module_context
{
  /* Handle returned by dlopen(). */
  void *dl_handle;
  /* Pointer to be used by the modules. */
  void *module_data;
  /* Array of hook functions befined by a single module.  Stored in the same
   * order as the global hooks. */
  module_hook_function hooks[nr_hooks];
};

static void close_module(struct plugin *m);
static bool call_module_hook(struct plugin *m, const char *hook_name);

/* Create a new module type plugin. */
struct plugin *open_module(const char *name, char **error)
{
  char *path;
  struct plugin *m = __allocate_plugin(name);
  struct module_context *context = ensure_malloc(sizeof *context);

  context->module_data = NULL;

  if (asprintf(&path, "%s/%s.so", VLOCK_MODULE_DIR, name) < 0) {
    *error = strdup("filename too long");
    goto path_error;
  }

  /* Test for access.  This must be done manually because vlock most likely
   * runs as a setuid executable and would otherwise override restrictions. */
  if (access(path, R_OK) < 0) {
    (void) asprintf(error, "%s: %s", path, strerror(errno));
    goto access_error;
  }

  /* Open the module as a shared library. */
  context->dl_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (context->dl_handle == NULL) {
    *error = strdup(dlerror());
    goto dlopen_error;
  }

  free(path);

  /* Load all the hooks.  Unimplemented hooks are NULL and will not be called later. */
  for (size_t i = 0; i < nr_hooks; i++)
    *(void **) (&context->hooks[i]) = dlsym(context->dl_handle, hooks[i].name);


  /* Load all dependencies.  Unspecified dependencies are NULL. */
  for (size_t i = 0; i < nr_dependencies; i++) {
    const char *(*dependency)[] = dlsym(context->dl_handle, dependency_names[i]);

    for (size_t j = 0; dependency != NULL && (*dependency)[j] != NULL; j++)
      list_append(m->dependencies[i], ensure_not_null(strdup((*dependency)[j]), "failed to copy string"));
  }

  m->context = context;
  m->close = close_module;
  m->call_hook = call_module_hook;

  return m;

dlopen_error:
access_error:
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
  free(context);
}

static bool call_module_hook(struct plugin *m, const char *hook_name)
{
  bool result = true;

  /* Find the right hook index. */
  for (size_t i = 0; i < nr_hooks; i++)
    if (strcmp(hooks[i].name, hook_name) == 0) {
      struct module_context *context = m->context;
      module_hook_function hook = context->hooks[i];
      
      if (hook != NULL)
        result = context->hooks[i](&context->module_data);

      break;
    }

  return result;
}

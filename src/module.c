#if !defined(__FreeBSD__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>

#include "plugin.h"

#include "module.h"

#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

struct plugin *open_module(const char *name, char **error)
{
  char *path;
  struct plugin *m = __allocate_plugin(name);

  if (asprintf(&path, "%s/%s.so", VLOCK_MODULE_DIR, name) < 0) {
    __destroy_plugin(m);
    *error = strdup("filename too long");
    return NULL;
  }

  if (access(path, R_OK) < 0) {
    (void) asprintf(error, "%s: %s", path, strerror(errno));
    free(path);
    __destroy_plugin(m);
    return NULL;
  }

  free(path);

  return m;
}

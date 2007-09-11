#include <stdio.h>
#include <module.h>
#include <dlfcn.h>

#include "plugin.h"

/* hard coded paths */
#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

Module::Module(string name) : Plugin(name)
{
  char *path;

  /* format the plugin path */
  if (asprintf(&path, "%s/%s.so", VLOCK_MODULE_DIR, name.c_str()) < 0)
    throw std::bad_alloc();

  /* load the plugin */
  dl_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (dl_handle == NULL)
    throw new string(dlerror());

  /* load the hooks, unimplemented hooks are NULL */
  for (size_t i = 0; i < ARRAY_SIZE(hooks); i++)
    *(void **) (&hooks[i]) = dlsym(dl_handle, hook_names[i]);

  /* load dependencies */
  for (size_t i = 0; i < ARRAY_SIZE(dependencies); i++) {
    const char *(*dependency)[] = (const char* (*)[])dlsym(dl_handle, dependency_names[i]);

    for (size_t j = 0; dependency != NULL && (*dependency)[j] != NULL; j++)
      dependencies[i].push_back((*dependency)[j]);
  }

}

Module::~Module()
{
  if (dl_handle != NULL)
    (void) dlclose(dl_handle);
}

#include <stdio.h>
#include <module.h>
#include <dlfcn.h>

#include <vector>
#include <iterator>

#include "plugin.h"

/* hard coded paths */
#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

// outsmart gcc to shut up warnings about conversions from void pointer to
// function pointer
typedef vlock_hook_fn (*hook_dlsym_t)(void *, const char *);

Module::Module(string name) : Plugin(name)
{
  char *path;

  this->ctx = NULL;

  /* format the plugin path */
  if (asprintf(&path, "%s/%s.so", VLOCK_MODULE_DIR, name.c_str()) < 0)
    throw std::bad_alloc();

  /* load the plugin */
  dl_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (dl_handle == NULL)
    throw new string(dlerror());

  /* load the hooks, unimplemented hooks are NULL */
  for (vector<string>::iterator it = hook_names.begin();
      it != hook_names.end(); it++)
    hooks[*it] = ((hook_dlsym_t)dlsym)(dl_handle, (*it).c_str());

  /* load dependencies */
  for (vector<string>::iterator it = dependency_names.begin();
      it != dependency_names.end(); it++) {
    const char *(*dependency)[] = (const char* (*)[])dlsym(dl_handle, (*it).c_str());

    for (size_t j = 0; dependency != NULL && (*dependency)[j] != NULL; j++)
      dependencies[*it].push_back((*dependency)[j]);
  }

}

Module::~Module()
{
  if (dl_handle != NULL)
    (void) dlclose(dl_handle);
}

bool Module::call_hook(string name)
{
  return true;
}

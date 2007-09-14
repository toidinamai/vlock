#include <stdio.h>
#include <module.h>
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <iterator>

#include "plugin.h"

/* hard coded module path */
#define VLOCK_MODULE_DIR PREFIX "/lib/vlock/modules"

// outsmart gcc to shut up warnings about conversions from void pointer to
// function pointer
typedef vlock_hook_fn (*hook_dlsym_t)(void *, const char *);

Module::Module(string name) : Plugin(name)
{
  char path[FILENAME_MAX];

  this->ctx = NULL;

  /* format the plugin path */
  if (snprintf(path, sizeof path, "%s/%s.so", VLOCK_MODULE_DIR, name.c_str()) > (ssize_t)sizeof path)
    throw PluginException("plugin '" + name + "' filename too long");

  if (access(path, R_OK | X_OK) != 0)
    throw PluginException(string(path) + ": " + strerror(errno));

  /* load the plugin */
  dl_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);

  if (dl_handle == NULL)
    throw PluginException(dlerror());

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

void Module::call_hook(string name)
{
  vlock_hook_fn hook = hooks[name];

  if (hook == NULL)
    return;

  if (!hook(&ctx))
    throw PluginException("error calling hook '" + name + "' for module '" + this->name + "'");
}

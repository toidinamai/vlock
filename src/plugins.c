#include "plugins.h"
#include "plugin.h"
#include "module.h"
#include "script.h"

bool load_plugin(const char *name)
{
  return true;
}

bool is_loaded(const char *name)
{
  return true;
}

bool resolve_dependencies(void)
{
  return true;
}

void unload_plugins(void)
{
}

bool plugin_hook(const char *hook)
{
  return true;
}

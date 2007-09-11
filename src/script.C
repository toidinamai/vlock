#include "script.h"

/* hard coded paths */
#define VLOCK_SCRIPT_DIR PREFIX "/lib/vlock/scripts"

Script::Script(string name) : Plugin(name)
{
  char *path;

  /* format the plugin path */
  if (asprintf(&path, "%s/%s", VLOCK_SCRIPT_DIR, name.c_str()) < 0)
    throw std::bad_alloc();
}

bool Script::call_hook(string name)
{
  return true;
}

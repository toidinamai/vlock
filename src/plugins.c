#include <stdio.h>
#include <stdlib.h>

#include "plugins.h"

#include "list.h"

#include "plugin.h"
#include "module.h"
#include "script.h"

static struct list *plugins;

static struct plugin *get_plugin(const char *name)
{
  return NULL;
}

bool load_plugin(const char *name)
{
  char *e1 = NULL;
  char *e2 = NULL;
  struct plugin *p = get_plugin(name);

  if (p != NULL)
    return true;

  p = open_module(name, &e1);

  if (p == NULL)
    p = open_script(name, &e2);

  if (p == NULL) {
    if (e1 != NULL) {
      fprintf(stderr, "vlock-plugins: %s\n", e1);
      free(e1);
    }
    if (e2 != NULL) {
      fprintf(stderr, "vlock-plugins: %s\n", e2);
      free(e2);
    }

    return false;
  }

  list_append(plugins, p);

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

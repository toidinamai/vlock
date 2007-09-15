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

void load_plugin(const char *name)
{
  char *e1 = NULL;
  char *e2 = NULL;
  struct plugin *p = get_plugin(name);

  if (p != NULL)
    return;

  p = open_module(name, &e1);

  if (p == NULL)
    p = open_script(name, &e2);

  if (p == NULL) {
    if (e1 == NULL && e2 == NULL) {
      fatal_error("vlock-plugins: error loading plugin '%s'\n", name);

    if (e1 != NULL) {
      fprintf(stderr, "vlock-plugins: error loading module '%s': %s\n", name, e1);
      free(e1);
    }
    if (e2 != NULL) {
      fprintf(stderr, "vlock-plugins: error loading script '%s': %s\n", name, e2);
      free(e2);
    }

    abort();
  }

  list_append(plugins, p);
}

void resolve_dependencies(void)
{
}

void unload_plugins(void)
{
}

void plugin_hook(const char *hook)
{
}

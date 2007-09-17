#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugins.h"

#include "list.h"

#include "plugin.h"
#include "module.h"
#include "script.h"
#include "util.h"

const char *dependency_names[nr_dependencies] = {
  "after",
  "before",
  "requires",
  "needs",
  "depends",
  "conflicts",
};

static void handle_vlock_start(const char * hook_name);
static void handle_vlock_end(const char * hook_name);
static void handle_vlock_save(const char * hook_name);
static void handle_vlock_save_abort(const char * hook_name);

const struct hook hooks[nr_hooks] = {
  { "vlock_start", handle_vlock_start },
  { "vlock_end", handle_vlock_end },
  { "vlock_save", handle_vlock_save },
  { "vlock_save_abort", handle_vlock_save_abort },
};

static struct list *plugins = &(struct list ){ NULL, NULL };

static struct plugin *get_plugin(const char *name)
{
  list_for_each(plugins, plugin_item) {
    struct plugin *p = plugin_item->data;
    if (strcmp(name, p->name) == 0)
      return p;
  }

  return NULL;
}

static struct plugin *__load_plugin(const char *name)
{
  char *e1 = NULL;
  char *e2 = NULL;
  struct plugin *p = get_plugin(name);

  if (p != NULL)
    return p;

  p = open_module(name, &e1);

  if (p == NULL)
    p = open_script(name, &e2);

  if (p == NULL) {
    if (e1 == NULL && e2 == NULL)
      fatal_error("vlock-plugins: error loading plugin '%s'", name);

    if (e1 != NULL) {
      fprintf(stderr, "vlock-plugins: error loading module '%s': %s", name, e1);
      free(e1);
    }
    if (e2 != NULL) {
      fprintf(stderr, "vlock-plugins: error loading script '%s': %s", name, e2);
      free(e2);
    }

    abort();
  }

  list_append(plugins, p);
  return p;
}

void load_plugin(const char *name)
{
  (void) __load_plugin(name);
}

static void __resolve_depedencies(void)
{
}

static void sort_plugins(void)
{
}

void resolve_dependencies(void)
{
  __resolve_depedencies();
  sort_plugins();
}

void unload_plugins(void)
{
}

void plugin_hook(const char *hook_name)
{
  for (size_t i = 0; i < nr_hooks; i++)
    if (strcmp(hook_name, hooks[i].name) == 0) {
      hooks[i].handler(hook_name);
      return;
    }

  fatal_error("vlock-plugins: invalid hook name '%s'", hook_name);
}

void handle_vlock_start(const char *hook_name)
{
}

void handle_vlock_end(const char *hook_name)
{
}

void handle_vlock_save(const char *hook_name)
{
}

void handle_vlock_save_abort(const char *hook_name)
{
}

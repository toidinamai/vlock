#include <stdlib.h>
#include <string.h>

#include "list.h"

#include "plugin.h"
#include "util.h"

/* Allocate a new plugin struct. */
struct plugin *__allocate_plugin(const char *name)
{
  struct plugin *p = ensure_malloc(sizeof *p);

  p->name = strdup(name);
  p->save_disabled = false;

  for (size_t i = 0; i < nr_dependencies; i++)
    p->dependencies[i] = list_new();

  return p;
}

/* Destroy the given plugin. (Internal version.) */
void __destroy_plugin(struct plugin *p)
{
  for (size_t i = 0; i < nr_dependencies; i++) {
    list_delete_for_each(p->dependencies[i], dependency_item)
      free(dependency_item->data);

    list_free(p->dependencies[i]);
  }

  free(p->name);
  free(p);
}

/* Destroy the given plugin. (External version.) */
void destroy_plugin(struct plugin *p)
{
  p->close(p);
  __destroy_plugin(p);
}

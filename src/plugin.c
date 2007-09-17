#include <stdlib.h>
#include <string.h>

#include "list.h"

#include "plugin.h"
#include "util.h"

struct plugin *__allocate_plugin(const char *name)
{
  struct plugin *p = ensure_malloc(sizeof *p);
  p->name = strdup(name);

  for (size_t i = 0; i < nr_dependencies; i++)
    p->dependencies[i] = list_new();

  return p;
}

static void free_dependency_list(struct list *dependency_list)
{
  list_free(dependency_list, free);
}

void __destroy_plugin(struct plugin *p)
{
  for (size_t i = 0; i < nr_dependencies; i++)
    list_free(p->dependencies[i], (list_free_item_function)free_dependency_list);

  free(p->name);
  free(p);
}

void destroy_plugin(struct plugin *p)
{
  p->close(p);
  __destroy_plugin(p);
}

#include "list.h"

#include "plugin.h"

struct list *dependency_names;

void __attribute__((constructor)) init_dependency_names(void)
{
  dependency_names = list_new();
  list_append(dependency_names, "after");
  list_append(dependency_names, "before");
  list_append(dependency_names, "requires");
  list_append(dependency_names, "needs");
  list_append(dependency_names, "depends");
  list_append(dependency_names, "conflicts");
}

void __attribute__((destructor)) uninit_dependency_names(void)
{
  list_free(dependency_names);
}

struct list *hook_names;

void __attribute__((constructor)) init_hook_names(void)
{
  hook_names = list_new();
  list_append(hook_names, "vlock_start");
  list_append(hook_names, "vlock_end");
  list_append(hook_names, "vlock_save");
  list_append(hook_names, "vlock_save_abort");
}

void __attribute__((destructor)) uninit_hook_names(void)
{
  list_free(hook_names);
}

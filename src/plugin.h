#include <stdbool.h>

#define NR_DEPENDENCIES 6
#define NR_HOOKS 4

struct list;

// list of dependency names
extern struct list *dependency_names;

// list of hook names
extern struct list *hook_names;

struct plugin
{
  const char *name;

  // dependencies
  struct list *dependencies;

  bool (*call_hook)(struct plugin *p, const char *name);
  void (*destroy)(struct plugin *p);

  void *context;
};

struct plugin *__allocate_plugin(const char *name);

void __destroy_plugin(struct plugin *p);
void destroy_plugin(struct plugin *p);

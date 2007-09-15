#include <stdbool.h>

#define NR_DEPENDENCIES 6
#define NR_HOOKS 4

struct list;

// array of dependency names
extern const char *dependency_names[NR_DEPENDENCIES];

// array of hook names, terminated by NULL
extern const char *hook_names[NR_HOOKS];

struct plugin
{
  const char *name;

  // dependencies
  struct list *after;
  struct list *before;
  struct list *requires;
  struct list *needs;
  struct list *depends;
  struct list *conflicts;

  bool (*call_hook)(struct plugin *p, const char *name);
};

struct plugin *allocate_plugin(const char *name);

void delete_plugin(struct plugin *p);

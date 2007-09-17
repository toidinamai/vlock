#include <stdbool.h>

// dependency names
#define nr_dependencies 6
extern const char *dependency_names[nr_dependencies];

struct hook
{
  const char *name;
  void (*handler)(const char *);
};

// hooks
#define nr_hooks 4
extern const struct hook hooks[nr_hooks];

struct plugin
{
  char *name;

  // dependencies
  struct list *dependencies[nr_dependencies];

  bool (*call_hook)(struct plugin *p, const char *name);
  void (*close)(struct plugin *p);

  void *context;
};

struct plugin *__allocate_plugin(const char *name);

void __destroy_plugin(struct plugin *p);

void destroy_plugin(struct plugin *p);

#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <list>
#include <string>

#undef ARRAY_SIZE
#define ARRAY_SIZE(x) ((sizeof (x) / sizeof (x[0])))

using namespace std;

/* function type for hooks */
typedef bool (*vlock_hook_fn)(void **);

/* vlock plugin */
class Plugin
{
public:
  /* name of the plugin */
  string name;

  /* plugin hook context */
  void *ctx;

  /* dependencies */
  list<string> dependencies[6];

  /* plugin hook functions */
  vlock_hook_fn hooks[4];

  // constructor
  Plugin(string name);

  virtual void call_hook(string name) = 0;
};

extern const char *hook_names[];
extern const char *dependency_names[];

#endif // _PLUGIN_H

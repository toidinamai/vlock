#include <string>
#include <map>
#include "plugin.h"

/* function type for hooks */
typedef bool (*vlock_hook_fn)(void **);

class Module : public Plugin
{
  /* path to the shared object file */
  string path;

  /* dl handle */
  void *dl_handle;

  /* plugin hook context */
  void *ctx;

  /* hooks */
  map<string, vlock_hook_fn> hooks;

public:
  Module(string name);
  ~Module();

  bool call_hook(string name);
};

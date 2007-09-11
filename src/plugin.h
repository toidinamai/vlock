#include <list>
#include <string>

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
};


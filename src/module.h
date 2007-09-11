#include <string>
#include "plugin.h"

class Module : public Plugin
{
public:
  /* path to the shared object file */
  string path;

  /* dl handle */
  void *dl_handle;

  Module(string name);
  ~Module();

  void call_hook(string name);
};

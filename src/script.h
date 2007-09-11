#include <string>
#include "plugin.h"

class Script : public Plugin
{
public:
  Script(string name);

  void call_hook(string name);
};

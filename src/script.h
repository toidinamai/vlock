#include <string>
#include "plugin.h"

class Script : public Plugin
{
public:
  Script(string name);

  bool call_hook(string name);
};

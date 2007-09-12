#include <unistd.h>
#include <string>
#include "plugin.h"

class Script : public Plugin
{
  // file descriptor for the child
  int fd;

  // pid of the child
  pid_t pid;

public:
  Script(string name);
  ~Script();

  bool call_hook(string name);
};

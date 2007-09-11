#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <list>
#include <map>
#include <vector>
#include <string>

#undef ARRAY_SIZE
#define ARRAY_SIZE(x) ((sizeof (x) / sizeof (x[0])))

using namespace std;

extern vector<string> dependency_names;
extern vector<string> hook_names;

/* vlock plugin */
class Plugin
{
public:
  /* name of the plugin */
  string name;

  /* dependencies */
  map<string, list<string> > dependencies;

  // constructor
  Plugin(string name);
  virtual ~Plugin();

  // hook
  virtual bool call_hook(string name) = 0;
};

#endif // _PLUGIN_H

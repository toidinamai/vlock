#ifndef _PLUGIN_H
#define _PLUGIN_H

#include <string>
#include <vector>
#include <list>
#include <map>

using std::string;
using std::vector;
using std::list;
using std::map;

extern vector<string> dependency_names;
extern vector<string> hook_names;

struct PluginException
{
  string reason;

  PluginException(string reason);
};

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
  virtual void call_hook(string name) = 0;
};

#endif // _PLUGIN_H

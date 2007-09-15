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

// list of dependency names
extern vector<string> dependency_names;
// list of hook names
extern vector<string> hook_names;

// recoverable error while loading or calling plugins
struct PluginException
{
  string reason;

  PluginException(string reason);
};

// non-recoverable error while loading or calling plugins
struct PluginError : public PluginException
{
};

// abstract base class representing a loaded plugin
class Plugin
{
public:
  // name of the plugin
  string name;

  // dependencies
  // keys are dependecy names, values are names of plugins depended on
  map<string, list<string> > dependencies;

  // constructor
  // Throws PluginException or PluginError on error.
  Plugin(string name);
  // destructor
  virtual ~Plugin();

  // Call the named hook.  Throws PluginException or PluginError on error.
  virtual void call_hook(string name) = 0;
};

#endif // _PLUGIN_H

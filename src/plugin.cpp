#include <vector>

#include "plugin.h"

/* dependency names */
vector<string> dependency_names;

void __attribute__((constructor)) init_dependency_names(void)
{
  dependency_names.push_back("after");
  dependency_names.push_back("before");
  dependency_names.push_back("requires");
  dependency_names.push_back("needs");
  dependency_names.push_back("depends");
  dependency_names.push_back("conflicts");
}

/* hook names */
vector<string> hook_names;

void __attribute__((constructor)) init_hook_names(void)
{
  hook_names.push_back("vlock_start");
  hook_names.push_back("vlock_end");
  hook_names.push_back("vlock_save");
  hook_names.push_back("vlock_save_abort");
}

// constructor
Plugin::Plugin(string name)
{
  this->name = name;
}

// destructor
Plugin::~Plugin()
{
}

PluginException::PluginException(string reason)
{
  this->reason = reason;
}

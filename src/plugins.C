/* plugins.c -- plugin routines for vlock,
 *              the VT locking program for linux
 *
 * This program is copyright (C) 2007 Frank Benkstein, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

#if !defined(__FreeBSD__) && !defined(_GNU_SOURCE)
/* for asprintf() */
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <list>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <functional>

#include "vlock.h"
#include "plugins.h"
#include "tsort.h"
#include "plugin.h"
#include "module.h"
#include "script.h"

using namespace std;

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

typedef bool (*hook_handler)(string);
static map<string, hook_handler> hook_handlers;

static bool handle_vlock_start(string hook_name);
static bool handle_vlock_end(string hook_name);
static bool handle_vlock_save(string hook_name);
static bool handle_vlock_save_abort(string hook_name);

static void __attribute__((constructor)) init_hook_handlers(void)
{
  hook_handlers["vlock_start"] = handle_vlock_start;
  hook_handlers["vlock_end"] = handle_vlock_end;
  hook_handlers["vlock_save"] = handle_vlock_save;
  hook_handlers["vlock_save_abort"] = handle_vlock_save_abort;
}

/* the list of plugins */
list<Plugin *> plugins;

// constructor
Plugin::Plugin(string name)
{
  this->name = name;
}

// destructor
Plugin::~Plugin()
{
}

struct name_matches : public unary_function<Plugin, bool>
{
  string name;
  name_matches(string name) { this->name = name; }
  bool operator() (Plugin *p) { return p->name == name; }
};

/* Get the plugin with the given name.  Returns the first plugin
 * with the given name or NULL if none could be found. */
static Plugin *get_plugin(string name)
{
  list<Plugin *>::iterator r = find_if(plugins.begin(), plugins.end(), name_matches(name));

  if (r != plugins.end())
    return *r;
  else
    return NULL;
}

/* Check if the given plugin is loaded. */
bool is_loaded(const char *name)
{
  list<Plugin *>::iterator r = find_if(plugins.begin(), plugins.end(), name_matches(name));

  return (r != plugins.end());
}

/* Same as open_plugin except that an old plugin is returned if there
 * already is one with the given name. */
static Plugin *__load_plugin(string name)
{
  list<Plugin *>::iterator r = find_if(plugins.begin(), plugins.end(), name_matches(name));

  if (r != plugins.end()) {
    return *r;
  } else {
    Plugin *p;

    try {
      p = new Module(name);
    }
    catch (...) {
      p = new Script(name);
    }

    plugins.push_back(p);

    return p;
  }
}

/* Same as __load_plugin except that true is returned on success and false on
 * error. */
bool load_plugin(const char *name)
{
  return __load_plugin(name) != NULL;
}

/* Unload all plugins */
void unload_plugins(void)
{
  while (!plugins.empty()) {
    Plugin *p = plugins.front();
    plugins.pop_front();
    delete p;
  }
}

/* Forward declaration */
static bool sort_plugins(void);

bool resolve_dependencies(void)
{
  list<Plugin *> required_plugins;

  /* load plugins that are required, this automagically takes care of plugins
   * that are required by the plugins loaded here because they are appended to
   * the end of the list */
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    for(list<string>::iterator it2 = (*it)->dependencies["requires"].begin();
        it2 != (*it)->dependencies["requires"].end(); it2++) {
      Plugin *d = __load_plugin(*it2);

      if (d == NULL)
        goto err;

      required_plugins.push_back(d);
    }
  }

  /* fail if a plugins that is needed is not loaded */
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    for(list<string>::iterator it2 = (*it)->dependencies["needs"].begin();
        it2 != (*it)->dependencies["needs"].end(); it2++) {
      Plugin *d = get_plugin(*it2);

      if (d == NULL) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n",
                (*it)->name.c_str(), (*it2).c_str());
        goto err;
      }

      required_plugins.push_back(d);
    }
  }

  /* unload plugins whose prerequisites are not present */
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end();) {
    bool dependencies_present = true;

    for(list<string>::iterator it2 = (*it)->dependencies["depends"].begin();
        it2 != (*it)->dependencies["depends"].end(); it2++) {
      Plugin *d = get_plugin(*it2);

      if (d != NULL)
        continue;

      dependencies_present = false;

      if (find(required_plugins.begin(), required_plugins.end(), *it) != required_plugins.end()) {
        fprintf(stderr, "vlock-plugins: %s does not work without %s\n",
                (*it)->name.c_str(), (*it2).c_str());
        goto err;
      }
    }

    if (dependencies_present) {
      it++;
    } else {
      Plugin *p = *it;
      it = plugins.erase(it);
      delete p;
    }
  }

  required_plugins.clear();

  /* fail if conflicting plugins are loaded */
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    for(list<string>::iterator it2 = (*it)->dependencies["conflicts"].begin();
        it2 != (*it)->dependencies["conflicts"].end(); it2++) {
      if (find_if(plugins.begin(), plugins.end(), name_matches(*it2)) != plugins.end()) {
        fprintf(stderr,
                "vlock-plugins: %s and %s cannot be loaded at the same time\n",
                (*it)->name.c_str(), (*it2).c_str());
        return false;
      }
    }
  }

  return sort_plugins();

err:
  return false;
}

/* A plugin may declare which plugins it must come before or after.  All those
 * relations together form a directed (acyclic) graph.  This graph is sorted
 * using a topological sort.
 */

/* Get all edges as specified by the all plugins' "after" and "before"
 * attributes. */
static list<Edge<Plugin *>*> *get_edges(void)
{
  list<Edge<Plugin *>*> *edges = new list<Edge<Plugin *>*>;

  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    Plugin *p = *it;
    /* p must come after these */
    list<string> predecessors = p->dependencies["after"];
    /* p must come before these */
    list<string> successors = p->dependencies["before"];

    for (list<string>::iterator it = successors.begin();
        it != successors.end(); it++) {
      list<Plugin *>::iterator r = find_if(plugins.begin(), plugins.end(), name_matches(*it));

      if (r != plugins.end()) {
        edges->push_back(new Edge<Plugin *>(p, *r));
      }
    }

    for (list<string>::iterator it = predecessors.begin();
        it != predecessors.end(); it++) {
      list<Plugin *>::iterator r = find_if(plugins.begin(), plugins.end(), name_matches(*it));

      if (r != plugins.end())
        edges->push_back(new Edge<Plugin *>(*r, p));
    }
  }

  return edges;
}

static bool sort_plugins(void)
{
  list<Edge<Plugin *>*> *edges = get_edges();
  bool result = tsort(plugins, *edges);

  if (!result) {
    fprintf(stderr, "vlock-plugins: circular dependencies detected:\n");

    for (list<Edge<Plugin *>*>::iterator it = edges->begin();
        it != edges->end(); it = edges->erase(it)) {
      fprintf(stderr, "\t%s\tmust come before\t%s\n", (*it)->predecessor->name.c_str(), (*it)->successor->name.c_str());
      delete *it;
    }
  }

  delete edges;

  return result;
}


/* Call the given plugin hook. */
bool plugin_hook(const char *hook_name)
{
  hook_handler h = hook_handlers["hook_name"];

  if (h == NULL) {
    fprintf(stderr, "vlock-plugins: unknown hook '%s'\n", hook_name);
    return false;
  } else {
    return h(hook_name);
  }
}

static bool handle_vlock_start(string hook_name)
{
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++)
    if (!(*it)->call_hook(hook_name))
      return false;

  return true;
}

static bool handle_vlock_end(string hook_name)
{
  for (list<Plugin *>::reverse_iterator it = plugins.rbegin();
      it != plugins.rend(); it--)
    (void) (*it)->call_hook(hook_name);

  return true;
}

/* Return true if at least one hook was called and all hooks were successful.
 * Does not continue after the first failing hook. */
static bool handle_vlock_save(string hook_name)
{
  bool result = false;

  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    result = (*it)->call_hook(hook_name);

    if (!result) {
      /* don't call again */
      // XXX: call vlock_save_abort, vlock_end, remove plugin
      break;
    }
  }

  return result;
}

static bool handle_vlock_save_abort(string hook_name)
{
  for (list<Plugin *>::reverse_iterator it = plugins.rbegin();
      it != plugins.rend(); it--) {
    if ((*it)->call_hook(hook_name)) {
      /* don't call again */
      // XXX: call vlock_end, remove plugin
    }
  }

  return true;
}

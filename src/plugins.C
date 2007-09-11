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

#undef ARRAY_SIZE
#define ARRAY_SIZE(x) ((sizeof (x) / sizeof (x[0])))

/* dependency names */
const char *dependency_names[] = {
  "after",
  "before",
  "requires",
  "needs",
  "depends",
  "conflicts",
};

/* hook names */
const char *hook_names[] = {
  "vlock_start",
  "vlock_end",
  "vlock_save",
  "vlock_save_abort",
};

static bool handle_vlock_start(int hook_index);
static bool handle_vlock_end(int hook_index);
static bool handle_vlock_save(int hook_index);
static bool handle_vlock_save_abort(int hook_index);

bool (*hook_handlers[])(int) = {
  handle_vlock_start,
  handle_vlock_end,
  handle_vlock_save,
  handle_vlock_save_abort,
};

/* the list of plugins */
list<Plugin *> plugins;

// constructor
Plugin::Plugin(string name)
{
  this->name = name;
  this->ctx = NULL;
}

static int __get_index(const char *a[], size_t l, const char *s) {
  for (size_t i = 0; i < l; i++)
    if (strcmp(a[i], s) == 0)
      return i;

  return -1;
}

#define get_index(a, s) __get_index(a, ARRAY_SIZE(a), s)

#define get_dependency(p, d) \
  ((p)->dependencies[get_index(dependency_names, (d))])

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

// #define SCRIPT_DEPENDENCY_ERROR ((struct List *) -1)
// 
// static struct List *get_script_dependency(const char *path, const char *name)
// {
//   return SCRIPT_DEPENDENCY_ERROR;
// }

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
    for(list<string>::iterator it2 = get_dependency(*it, "requires").begin();
        it2 != get_dependency(*it, "requires").end(); it2++) {
      Plugin *d = __load_plugin(*it2);

      if (d == NULL)
        goto err;

      required_plugins.push_back(d);
    }
  }

  /* fail if a plugins that is needed is not loaded */
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    for(list<string>::iterator it2 = get_dependency(*it, "needs").begin();
        it2 != get_dependency(*it, "needs").end(); it2++) {
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

    for(list<string>::iterator it2 = get_dependency(*it, "depends").begin();
        it2 != get_dependency(*it, "depends").end(); it2++) {
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
    for (list<string>::iterator it2 = get_dependency(*it, "conflicts").begin();
        it2 != get_dependency(*it, "depends").end(); it2++) {
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
    list<string> predecessors = get_dependency(p, "after");
    /* p must come before these */
    list<string> successors = get_dependency(p, "before");

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
  int hook_index = get_index(hook_names, hook_name);

  if (hook_index < 0) {
    fprintf(stderr, "vlock-plugins: unknown hook '%s'\n", hook_name);
    return false;
  } else {
    return hook_handlers[hook_index] (hook_index);
  }
}

static bool handle_vlock_start(int hook_index)
{
  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    Plugin *p = *it;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL)
      if (!hook(&p->ctx))
        return false;
  }

  return true;
}

static bool handle_vlock_end(int hook_index)
{
  for (list<Plugin *>::reverse_iterator it = plugins.rbegin();
      it != plugins.rend(); it--) {
    Plugin *p = *it;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL)
      (void) hook(&p->ctx);
  }

  return true;
}

/* Return true if at least one hook was called and all hooks were successful.
 * Does not continue after the first failing hook. */
static bool handle_vlock_save(int hook_index)
{
  bool result = false;

  for (list<Plugin *>::iterator it = plugins.begin();
      it != plugins.end(); it++) {
    Plugin *p = *it;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL) {
      result = hook(&p->ctx);

      if (!result) {
        /* don't call again */
        p->hooks[hook_index] = NULL;
        result = false;
        break;
      }
    }
  }

  return result;
}

static bool handle_vlock_save_abort(int hook_index)
{
  for (list<Plugin *>::reverse_iterator it = plugins.rbegin();
      it != plugins.rend(); it--) {
    Plugin *p = *it;
    vlock_hook_fn hook = p->hooks[hook_index];

    if (hook != NULL) {
      int vlock_save_index = get_index(hook_names, "vlock_save");

      if (!hook(&p->ctx) || p->hooks[vlock_save_index] == NULL) {
        /* don't call again */
        p->hooks[hook_index] = NULL;
        p->hooks[vlock_save_index] = NULL;
      }
    }
  }

  return true;
}

#include <iterator>
#include <list>
#include "tsort.h"

/* Check if the given node has no incoming edges. */
template <class T>
static bool is_zero(T& node, std::list<Edge<T>*>& edges)
{
  for (typename std::list<Edge<T>*>::iterator it = edges.begin();
      it != edges.end(); it++)
    if ((*it)->successor == node)
      return false;

  return true;
}

template <class T>
static bool list_remove(std::list<T> l, T i)
{
  for (typename std::list<T>::iterator it = l.begin(); it != l.end(); it++)
    if (*it == i) {
      l.erase(it);
      return true;
    }

  return false;
}


/* Get all nodes (plugins) with no incoming edges. */
template <class T>
static std::list<T> *get_zeros(std::list<T>& nodes, std::list<Edge<T>*>& edges)
{
  std::list<T> *zeros = new std::list<T>(nodes);

  for (typename std::list<Edge<T>*>::iterator it = edges.begin();
      !zeros.empty() && it != edges.end(); it++)
    zeros = list_remove(zeros, (*it)->successor);

  return zeros;
}


/* Sort all nodes with a topological sort.  Returns a list of sorted nodes if
 * sorting was successful and NULL if cycles were found.  If sorting was
 * successful, all edges are deleted.  If cycles were found the at least the
 * edges beloning to the cycles are left intact.
 *
 * Algorithm:
 * http://en.wikipedia.org/w/index.php?title=Topological_sorting&oldid=153157450#Algorithms
 */
template<class T> bool tsort(std::list<T>& nodes, std::list<Edge<T>*>& edges)
{
  std::list<T> *sorted_nodes = new std::list<T>;
  std::list<T> *zeros = get_zeros(nodes, *edges);
  bool result;

  while (!zeros->empty()) {
    T zero = zeros->front();
    zeros->pop_front();

    sorted_nodes->push_back(zeros);

    for (typename std::list<Edge<T>*>::iterator it = edges.begin();
        it != edges.end();) {
      if ((*it)->predecessor != zero) {
        it++;
        continue;
      } else {
        list_remove(edges, *it);

        if (is_zero((*it)->successor, edges))
          zeros.push_back((*it)->successor);

        delete *it;
      }
    }
  }

  result = edges.empty();

  if (result) {
    nodes.clear();
    nodes.insert(nodes.begin(), sorted_nodes.begin(), sorted_nodes.end());
  }

  delete sorted_nodes;
  return result;
}

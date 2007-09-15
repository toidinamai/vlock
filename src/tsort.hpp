#include <iterator>
#include <list>

using std::list;

/* An edge of the graph, specifying that predecessor must come before
 * successor. */
template<class T> struct Edge {
  T predecessor;
  T successor;

  Edge(T p, T s) { predecessor = p; successor = s; }
};

/* For the given digraph, generate a topological sort of the nodes.
 *
 * Sorts the list and deletes all edges.  If there are circles found in the
 * graph or there are edges that have no corresponding nodes the erroneous
 * edges are left. */
template<class T> bool tsort(list<T>& nodes, list<Edge<T> >& edges);


/* Check if the given node has no incoming edges. */
template <class T>
bool is_zero(T& node, list<Edge<T> >& edges)
{
  for (typename list<Edge<T> >::iterator it = edges.begin();
      it != edges.end(); it++)
    if ((*it).successor == node)
      return false;

  return true;
}

/* Get all nodes (plugins) with no incoming edges. */
template <class T>
static list<T> *get_zeros(list<T>& nodes, list<Edge<T> >& edges)
{
  list<T> *zeros = new list<T>(nodes);

  for (typename list<Edge<T> >::iterator it = edges.begin();
      !zeros->empty() && it != edges.end(); it++)
    zeros->remove((*it).successor);

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
template<class T> bool tsort(list<T>& nodes, list<Edge<T> >& edges)
{
  list<T> *sorted_nodes = new list<T>;
  list<T> *zeros = get_zeros(nodes, edges);
  bool result;

  // while there are zeros left
  while (!zeros->empty()) {
    // take the first zero
    T zero = zeros->front();
    // remove it from the list
    zeros->pop_front();
    // and add it to the list of sorted nodes
    sorted_nodes->push_back(zero);

    // look at all edges
    for (typename list<Edge<T> >::iterator it = edges.begin();
        it != edges.end();) {
      // where this zero is the predecessor
      if ((*it).predecessor == zero) {
        Edge<T> tmp = *it;
        // delete those edges
        it = edges.erase(it);

        // if the successor of the edge now is a zero
        if (is_zero(tmp.successor, edges))
          // add it to the list
          zeros->push_back(tmp.successor);
      } else {
        it++;
      }
    }
  }

  result = edges.empty();

  if (result)
    nodes.assign(sorted_nodes->begin(), sorted_nodes->end());

  delete sorted_nodes;
  return result;
}

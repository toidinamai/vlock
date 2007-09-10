#include <list>

/* An edge of the graph, specifying that predecessor must come before
 * successor. */
template<class T> struct Edge {
  T predecessor;
  T successor;
};

/* For the given digraph, generate a topological sort of the nodes.
 *
 * Sorts the list and deletes all edges.  If there are circles found in the
 * graph or there are edges that have no corresponding nodes the erroneous
 * edges are left. */
template<class T> bool tsort(std::list<T>& nodes, std::list<Edge<T>*>& edges);

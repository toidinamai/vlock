#include <stdbool.h>

struct list;

/* An edge of the graph, specifying that predecessor must come before
 * successor. */
struct edge {
  void *predecessor;
  void *successor;
};

/* For the given digraph, generate a topological sort of the nodes.
 *
 * Sorts the list and deletes all edges.  If there are circles found in the
 * graph or there are edges that have no corresponding nodes the erroneous
 * edges are left. */
bool tsort(struct list *nodes, struct list *edges);

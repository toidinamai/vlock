#ifdef __cplusplus
extern "C" {
#endif

struct List;

/* For the given digraph, generate a topological sort of the nodes.
 *
 * Returns a list of sorted nodes and deletes all edges.  If there are circles
 * found in the graph or there are edges that have no corresponding nodes, NULL
 * is returned and the erroneous edges are left. */
struct List *tsort(struct List *nodes, struct List **edges);

/* An edge of the graph, specifying that predecessor must come before
 * successor. */
struct Edge {
  void *predecessor;
  void *successor;
};

#ifdef __cplusplus
}
#endif

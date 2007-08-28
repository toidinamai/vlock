#include <stdlib.h>

#include <glib.h>

#include "tsort.h"

/* Check if the given node has no incoming edges. */
static int is_zero(void *node, GList *edges) {
  for (GList *item = g_list_first(edges); item != NULL; item = g_list_next(item)) {
    struct Edge *edge = item->data;

    if (edge->successor == node)
      return 0;
  }

  return 1;
}


/* Get all nodes (plugins) with no incoming edges. */
static GList *get_zeros(GList *nodes, GList *edges) {
  GList *zeros = g_list_copy(nodes);

  for (GList *item = g_list_first(edges); item != NULL && g_list_first(zeros) != NULL; item = g_list_next(item)) {
    struct Edge *edge = item->data;

    zeros = g_list_remove(zeros, edge->successor);
  }

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
GList *tsort(GList *nodes, GList **edges) {
  GList *sorted_nodes = NULL;
  GList *zeros = get_zeros(nodes, *edges);

  for (GList *item = g_list_first(zeros); item != NULL; item = g_list_first(zeros)) {
    void *node = item->data;

    sorted_nodes = g_list_append(sorted_nodes, node);
    zeros = g_list_remove(zeros, node);

    for (GList *item = g_list_first(*edges); item != NULL;) {
      struct Edge *edge = item->data;
      item = g_list_next(item);

      if (edge->predecessor != node)
        continue;

      *edges = g_list_remove(*edges, edge);

      if (is_zero(edge->successor, *edges))
        zeros = g_list_append(zeros, edge->successor);

      free(edge);
    }
  }

  if (g_list_length(*edges) == 0) {
    return sorted_nodes;
  } else {
    g_list_free(sorted_nodes);
    return NULL;
  }
}

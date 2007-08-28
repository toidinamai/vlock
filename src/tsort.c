#include <stdlib.h>

#include "tsort.h"
#include "list.h"

/* Check if the given node has no incoming edges. */
static int is_zero(void *node, struct List *edges) {
  list_for_each(edges, item) {
    struct Edge *edge = item->data;

    if (edge->successor == node)
      return 0;
  }

  return 1;
}


/* Get all nodes (plugins) with no incoming edges. */
static struct List *get_zeros(struct List *nodes, struct List *edges) {
  struct List *zeros = list_copy(nodes);

  list_for_each(edges, item) {
    struct Edge *edge = item->data;

    zeros = list_remove(zeros, edge->successor);

    if (zeros == NULL)
      break;
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
struct List *tsort(struct List *nodes, struct List **edges) {
  struct List *sorted_nodes = NULL;
  struct List *zeros = get_zeros(nodes, *edges);

  while (zeros != NULL) {
    void *node = zeros->data;

    sorted_nodes = list_append(sorted_nodes, node);
    zeros = list_delete_link(zeros, zeros);

    for (struct List *item = list_first(*edges); item != NULL;) {
      struct Edge *edge = item->data;
      item = list_next(item);

      if (edge->predecessor != node)
        continue;

      *edges = list_remove(*edges, edge);

      if (is_zero(edge->successor, *edges))
        zeros = list_append(zeros, edge->successor);

      free(edge);
    }
  }

  if (*edges == NULL) {
    return sorted_nodes;
  } else {
    list_free(sorted_nodes);
    return NULL;
  }
}

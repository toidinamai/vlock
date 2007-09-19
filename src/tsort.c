/* tsort.c -- topological sort for vlock, the VT locking program for linux
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

#include <stdlib.h>

#include "list.h"

#include "tsort.h"

/* Get the zeros of the graph, i.e. nodes with no incoming edges. */
static struct list *get_zeros(struct list *nodes, struct list *edges)
{
  struct list *zeros = list_copy(nodes);

  list_for_each(edges, edge_item) {
    struct edge *e = edge_item->data;
    list_delete(zeros, e->successor);
  }

  return zeros;
}

/* Check if the given node is a zero. */
static bool is_zero(void *node, struct list *edges)
{
  list_for_each(edges, edge_item) {
    struct edge *e = edge_item->data;

    if (e->successor == node)
      return false;
  }

  return true;
}

/* For the given directed graph, generate a topological sort of the nodes.
 *
 * Sorts the list and deletes all edges.  If there are circles found in the
 * graph or there are edges that have no corresponding nodes the erroneous
 * edges are left.
 *
 * The algorithm is taken from the Wikipedia:
 *
 * http://en.wikipedia.org/w/index.php?title=Topological_sorting&oldid=153157450#Algorithms
 *
 */
void tsort(struct list *nodes, struct list *edges)
{
  struct list *sorted_nodes = list_new();
  /* Retrieve all zeros. */
  struct list *zeros = get_zeros(nodes, edges);

  /* While the list of zeros is not empty, take the first zero and remove it
   * and ...  */
  list_delete_for_each(zeros, zero_item) {
    void *zero = zero_item->data;
    /* ... add it to the list of sorted nodes. */
    list_append(sorted_nodes, zero);

    /* Then look at each edge ... */
    list_for_each_manual(edges, edge_item) {
      struct edge *e = edge_item->data;

      /* ... that has this zero as its predecessor ... */
      if (e->predecessor == zero) {
        /* ... and remove it. */
        edge_item = list_delete_item(edges, edge_item);

        /* If the successor has become a zero now ... */
        if (is_zero(e->successor, edges))
          /* ... add it to the list of zeros. */
          list_append(zeros, e->successor);

        free(e);
      } else {
        edge_item = edge_item->next;
      }
    }
  }

  /* If all edges were deleted the algorithm was successful. */
  if (list_is_empty(edges)) {
    /* Switch the given list of nodes for the list of sorted nodes. */
    struct list_item *first = sorted_nodes->first;
    struct list_item *last = sorted_nodes->last;

    sorted_nodes->first = nodes->first;
    sorted_nodes->last = nodes->last;

    nodes->first = first;
    nodes->last = last;
  }

  list_free(sorted_nodes);
}

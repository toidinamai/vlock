#include <stdlib.h>

#include "list.h"

#include "tsort.h"

static struct list *get_zeros(struct list *nodes, struct list *edges)
{
  struct list *zeros = list_copy(nodes);

  list_for_each(edges, edge_item) {
    struct edge *e = edge_item->data;
    list_delete(zeros, e->successor);
  }

  return zeros;
}

static bool is_zero(void *node, struct list *edges)
{
  list_for_each(edges, edge_item) {
    struct edge *e = edge_item->data;

    if (e->successor == node)
      return false;
  }

  return true;
}

void tsort(struct list *nodes, struct list *edges)
{
  struct list *sorted_nodes = list_new();
  struct list *zeros = get_zeros(nodes, edges);

  list_delete_for_each(zeros, zero_item) {
    void *zero = zero_item->data;

    list_append(sorted_nodes, zero);

    list_for_each_manual(edges, edge_item) {
      struct edge *e = edge_item->data;

      if (e->predecessor == zero) {
        edge_item = list_delete_item(edges, edge_item);

        if (is_zero(e->successor, edges))
          list_append(zeros, e->successor);

        free(e);
      } else {
        edge_item = edge_item->next;
      }
    }
  }

  if (list_is_empty(edges)) {
    struct list_item *first = sorted_nodes->first;
    struct list_item *last = sorted_nodes->last;

    sorted_nodes->first = nodes->first;
    sorted_nodes->last = nodes->last;

    nodes->first = first;
    nodes->last = last;
  }

  list_free(sorted_nodes);
}

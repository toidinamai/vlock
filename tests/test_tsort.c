#include <stdlib.h>

#include <glib.h>

#include <CUnit/CUnit.h>

#include "tsort.h"

#include "test_tsort.h"

#define A ((void *)1)
#define B ((void *)2)
#define C ((void *)3)
#define D ((void *)4)
#define E ((void *)5)
#define F ((void *)6)
#define G ((void *)7)
#define H ((void *)8)

void test_tsort(void)
{
  GList *list = NULL;
  GList *edges = NULL;
  GList *faulty_edges = NULL;
  GList *sorted_list;

  list = g_list_append(list, A);
  list = g_list_append(list, B);
  list = g_list_append(list, C);
  list = g_list_append(list, D);
  list = g_list_append(list, E);
  list = g_list_append(list, F);
  list = g_list_append(list, G);
  list = g_list_append(list, H);

  /* Edges:
   *
   *  E
   *  |
   *  B C D   H
   *   \|/    |
   *    A   F G
   */
  edges = g_list_append(edges, make_edge(A, B));
  edges = g_list_append(edges, make_edge(A, C));
  edges = g_list_append(edges, make_edge(A, D));
  edges = g_list_append(edges, make_edge(B, E));
  edges = g_list_append(edges, make_edge(G, H));

  sorted_list = tsort(list, &edges);

  CU_ASSERT(edges == NULL);

  CU_ASSERT_PTR_NOT_NULL(sorted_list);

  CU_ASSERT_EQUAL(g_list_length(list), g_list_length(sorted_list));

  /* Check that all items from the original list are in the
   * sorted list. */
  for (GList *item = list; item != NULL; item = g_list_next(item))
    CU_ASSERT_PTR_NOT_NULL(g_list_find(sorted_list, item->data));

  CU_ASSERT(g_list_index(list, A) < g_list_index(list, B));
  CU_ASSERT(g_list_index(list, A) < g_list_index(list, C));
  CU_ASSERT(g_list_index(list, A) < g_list_index(list, D));

  CU_ASSERT(g_list_index(list, B) < g_list_index(list, E));

  CU_ASSERT(g_list_index(list, G) < g_list_index(list, H));

  /* Faulty edges: same as above but F wants to be below A and above E. */
  faulty_edges = g_list_append(faulty_edges, make_edge(A, B));
  faulty_edges = g_list_append(faulty_edges, make_edge(A, C));
  faulty_edges = g_list_append(faulty_edges, make_edge(A, D));
  faulty_edges = g_list_append(faulty_edges, make_edge(B, E));
  faulty_edges = g_list_append(faulty_edges, make_edge(E, F));
  faulty_edges = g_list_append(faulty_edges, make_edge(F, A));
  faulty_edges = g_list_append(faulty_edges, make_edge(G, H));

  CU_ASSERT_PTR_NULL(tsort(list, &faulty_edges));

  CU_ASSERT(g_list_length(faulty_edges) > 0);

  while (faulty_edges != NULL) {
    free(faulty_edges->data);
    faulty_edges = g_list_delete_link(faulty_edges, faulty_edges);
  }

  g_list_free(sorted_list);
  g_list_free(edges);
  g_list_free(faulty_edges);
  g_list_free(list);
}

CU_TestInfo tsort_tests[] = {
  { "test_tsort", test_tsort },
  CU_TEST_INFO_NULL,
};

#include <CUnit/CUnit.h>

#include "list.h"

#include "test_list.h"

void test_list_new(void)
{
  struct list *l = list_new();

  CU_ASSERT_PTR_NOT_NULL_FATAL(l);
  CU_ASSERT_PTR_NULL(l->first);
  CU_ASSERT_PTR_NULL(l->last);

  list_free(l);
}

void test_list_copy(void)
{
  struct list *l = list_new();
  struct list *m;

  list_append(l, (void *)1);
  list_append(l, (void *)2);
  list_append(l, (void *)3);

  m = list_copy(l);

  CU_ASSERT_EQUAL(list_length(l), list_length(m));
  CU_ASSERT_PTR_NOT_EQUAL(l, m);
  CU_ASSERT_PTR_NOT_EQUAL(l->first, m->first);
  CU_ASSERT_PTR_NOT_EQUAL(l->last, m->last);

  for (struct list_item *item_l = l->first, *item_m = m->first;
      item_l != NULL && item_m != NULL;
      item_l = item_l->next, item_m = item_m->next)
    CU_ASSERT_PTR_EQUAL(item_l->data, item_m->data);

  list_free(m);
  list_free(l);
}

CU_TestInfo list_tests[] = {
  { "test_list_new", test_list_new },
  { "test_list_copy", test_list_copy },
  CU_TEST_INFO_NULL,
};

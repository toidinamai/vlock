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

CU_TestInfo list_tests[] = {
  { "test_list_new", test_list_new },
  CU_TEST_INFO_NULL,
};

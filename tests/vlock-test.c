#include <stdlib.h>
#include <stdio.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "test_list.h"

CU_SuiteInfo vlock_test_suites[] = {
  { "test_list" , NULL, NULL, list_tests },
  CU_SUITE_INFO_NULL,
};

int main(int __attribute__((unused)) argc, const char *argv[])
{
  if (CU_initialize_registry() != CUE_SUCCESS) {
    fprintf(stderr, "%s: CUnit initialization failed\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (CU_register_suites(vlock_test_suites) != CUE_SUCCESS) {
    fprintf(stderr, "%s: registering test suites failed: %s\n", argv[0], CU_get_error_msg());
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);

  if (CU_basic_run_tests() != CUE_SUCCESS) {
    fprintf(stderr, "%s: running tests failed\n", argv[0]);
    goto error;
  }

  if (CU_get_number_of_tests_failed() > 0)
    goto error;

  CU_cleanup_registry();
  exit(EXIT_SUCCESS);

error:
  CU_cleanup_registry();
  exit(EXIT_FAILURE);
}

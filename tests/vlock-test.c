#include <stdlib.h>
#include <stdio.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

int main(int __attribute__((unused)) argc, const char *argv[])
{
  if (CU_initialize_registry() != CUE_SUCCESS) {
    fprintf(stderr, "%s: CUnit initialization failed\n", argv[0]);
    exit(EXIT_FAILURE);
  }

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

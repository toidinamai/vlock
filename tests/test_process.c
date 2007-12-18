#include <sys/types.h>
#include <unistd.h>

#include <CUnit/CUnit.h>

#include "process.h"

#include "test_process.h"

void test_wait_for_death(void)
{
  pid_t pid = fork();

  if (pid == 0) {
    usleep(10000);
    execl("/bin/true", "/bin/true", NULL);
    _exit(1);
  }

  CU_ASSERT(!wait_for_death(pid, 0, 2000));
  CU_ASSERT(wait_for_death(pid, 0, 20000));
}

CU_TestInfo process_tests[] = {
  { "test_wait_for_death", test_wait_for_death },
  CU_TEST_INFO_NULL,
};

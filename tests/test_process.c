#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

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

void test_ensure_death(void)
{
  pid_t pid = fork();

  if (pid == 0) {
    signal(SIGTERM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    execl("/bin/true", "/bin/true", NULL);
    _exit(0);
  }

  ensure_death(pid);

  CU_ASSERT(waitpid(pid, NULL, WNOHANG) < 0);
  CU_ASSERT(errno == ECHILD);
}

CU_TestInfo process_tests[] = {
  { "test_wait_for_death", test_wait_for_death },
  { "test_ensure_death", test_ensure_death },
  CU_TEST_INFO_NULL,
};

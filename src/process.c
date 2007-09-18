#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "process.h"

static void ignore_sigalarm(int __attribute__((unused)) signum)
{
  // ignore
}

bool wait_for_death(pid_t pid, long sec, long usec)
{
  int status;
  struct sigaction act, oldact;
  struct itimerval timer, otimer;
  bool result;

  // ignore SIGALRM
  sigemptyset(&act.sa_mask);
  act.sa_handler = ignore_sigalarm;
  act.sa_flags = 0;
  sigaction(SIGALRM, &act, &oldact);

  // initialize timer
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  timer.it_value.tv_sec = sec;
  timer.it_value.tv_usec = usec;

  // set timer
  setitimer(ITIMER_REAL, &timer, &otimer);

  // wait until interupted by timer
  result = (waitpid(pid, &status, 0) == pid);

  // restore signal handler
  sigaction(SIGALRM, &oldact, NULL);

  // restore timer
  setitimer(ITIMER_REAL, &otimer, NULL);

  return result;
}

void ensure_death(pid_t pid)
{
  int status;

  // look if already dead
  if (waitpid(pid, &status, WNOHANG) == pid)
    return;

  // kill!!!
  (void) kill(pid, SIGTERM);

  // wait 500ms for death
  if (wait_for_death(pid, 0, 500000L))
    return;

  // kill harder!!!
  (void) kill(pid, SIGKILL);
  (void) kill(pid, SIGCONT);

  // wait until dead
  (void) waitpid(pid, &status, 0);
}

void close_all_fds(void)
{
  struct rlimit r;
  int maxfd;

  // get the maximum number of file descriptors
  if (getrlimit(RLIMIT_NOFILE, &r) == 0)
    maxfd = r.rlim_cur;
  else
    // hopefully safe default
    maxfd = 1024;

  // close all file descriptors except STDIN_FILENO, STDOUT_FILENO and
  // STDERR_FILENO
  for (int i = 0; i < maxfd; i++) {
    switch (i) {
      case STDIN_FILENO:
      case STDOUT_FILENO:
      case STDERR_FILENO:
        break;
      default:
        (void) close(i);
    }
  }
}

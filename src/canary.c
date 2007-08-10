#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

static int canary_pid;

void canary_died(int __attribute__((__unused__)) signum) {
  int status;

  if (waitpid(canary_pid, &status, 0) == canary_pid) {
    if (WIFSIGNALED(status)) {
      (void) raise(WTERMSIG(status));
    } else {
      /* shouldn't happen */
    }
  }
}

void launch_canary(void) {
  struct sigaction sa;
  int p[2];

  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  sa.sa_handler = canary_died;
  (void) sigemptyset(&(sa.sa_mask));
  (void) sigaction(SIGCHLD, &sa, NULL);
  (void) pipe(p);

  canary_pid = fork();

  if (canary_pid == 0) {
    /* child */
    char c;
    sigset_t signals;
    (void) sigemptyset(&signals);

    /* don't block any signals */
    (void) sigprocmask(SIG_SETMASK, &signals, NULL);

    /* start own session */
    (void) setsid();

    /* close file descriptors */
    (void) close(STDIN_FILENO);
    (void) close(STDOUT_FILENO);
    (void) close(STDERR_FILENO);
    (void) close(p[1]);

    /* change uid */
    (void) setuid(getuid());

    /* wait for death */
    (void) read(p[0], &c, 1);
    
    _exit(0);
  }

  (void) close(p[0]);
}

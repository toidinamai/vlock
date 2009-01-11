#define _GNU_SOURCE
#include <stdio.h>

#include <signal.h>

#include <string.h>

void invoke_atexit_functions(void);

static void terminate(int signum)
{
  invoke_atexit_functions();
  fprintf(stderr, "vlock: Killed by signal %d (%s)!\n", signum, strsignal(signum));
  raise(signum);
}

void install_signal_handlers(void)
{
  struct sigaction sa;

  /* Ignore some signals. */
  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = SIG_IGN;
  (void) sigaction(SIGTSTP, &sa, NULL);

  /* Handle termination signals.  None of these should be delivered in a normal
   * run of the program because terminal signals (INT, QUIT) are disabled
   * below. */
  sa.sa_flags = SA_RESETHAND;
  sa.sa_handler = terminate;
  (void) sigaction(SIGINT, &sa, NULL);
  (void) sigaction(SIGQUIT, &sa, NULL);
  (void) sigaction(SIGTERM, &sa, NULL);
  (void) sigaction(SIGHUP, &sa, NULL);
  (void) sigaction(SIGABRT, &sa, NULL);
  (void) sigaction(SIGSEGV, &sa, NULL);
}

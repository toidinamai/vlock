#define _GNU_SOURCE
#include <stdio.h>

#include <signal.h>

#include <string.h>

#include "signals.h"
#include "util.h"

static const char *termination_blurb =
// XXX: reformat, bullet points etc.
"\n"
"vlock caught a signal and will now terminate.\n"
"It is very likely that the reason for this\n"
"is an error in the program.  If you believe\n"
"this to be the case please send and email to\n"
"the author.  Please include as much information\n"
"as possible about your system and the circum-\n"
"stances that led to the problem.  Do not forget\n"
"to mention the version number of vlock.  Please\n"
"include the word \"vlock\" in the subject of\n"
"your email.\n"
"\n"
"Frank Benkstein <frank-vlock@benkstein.net>\n"
"\n"
;

static void terminate(int signum)
{
  vlock_invoke_atexit();
  fprintf(stderr, "vlock: Killed by signal %d (%s)!\n", signum, strsignal(signum));
  fputs(termination_blurb, stderr);
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

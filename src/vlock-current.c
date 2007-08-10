/* vlock-current.c -- locking routine for vlock,
 *                    the VT locking program for linux
 *
 * This program is copyright (C) 2007 Frank Benkstein, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pwd.h>

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include "vlock.h"

/* Lock the current terminal until proper authentication is received. */
int main(void) {
  char user[40];
  char *vlock_message;
  char *vlock_prompt_timeout;
  struct timespec timeout;
  struct timespec *timeout_p = NULL;
  struct termios term, term_bak;
  struct sigaction sa;
  /* get the user id */
  uid_t uid = getuid();
  /* get the user name from the environment */
  char *envuser = getenv("USER"); 

  /* ignore some signals */
  /* these signals shouldn't be delivered anyway, because
   * terminal signals are disabled below */
  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = SIG_IGN;
  (void) sigaction(SIGINT, &sa, NULL);
  (void) sigaction(SIGQUIT, &sa, NULL);
  (void) sigaction(SIGTSTP, &sa, NULL);

  if (uid > 0 || envuser == NULL) {
    errno = 0;

    /* get the password entry */
    struct passwd *pw = getpwuid(uid);

    if (pw == NULL) {
      if (errno != 0)
        perror("vlock-current: getpwuid() failed");
      else
        fprintf(stderr, "vlock-current: getpwuid() failed\n");

      exit (111);
    }

    /* copy the username */
    strncpy(user, pw->pw_name, sizeof user - 1);
    user[sizeof user - 1] = '\0';
  } else {
    /* copy the username */
    strncpy(user, envuser, sizeof user - 1);
    user[sizeof user - 1] = '\0';
  }

  /* get the vlock message from the environment */
  vlock_message = getenv("VLOCK_MESSAGE");

  /* get the timeout from the environment */
  vlock_prompt_timeout = getenv("VLOCK_PROMPT_TIMEOUT");

  if (vlock_prompt_timeout != NULL) {
    char *n;
    timeout.tv_sec = strtol(vlock_prompt_timeout, &n, 10);
    timeout.tv_nsec = 0;


    if (*n == '\0' && timeout.tv_sec > 0)
      timeout_p = &timeout;
  }

  /* disable terminal echoing and signals */
  if (tcgetattr(STDIN_FILENO, &term) == 0) {
    term_bak = term;
    term.c_lflag &= ~(ECHO|ISIG);
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);
  }

  for (;;) {
    int c;

    /* clear the screen */
    fprintf(stderr, CLEAR_SCREEN);

    if (vlock_message)
      /* print vlock message */
      fprintf(stderr, "%s\n", vlock_message);

    /* wait for enter to be pressed */
    fprintf(stderr, "Please press [ENTER] to unlock.\n");
    while ((c = fgetc(stdin)) != '\n')
      if (c == EOF)
        clearerr(stdin);

    if (auth(user, timeout_p))
      break;
    else
      sleep(1);

#ifndef NO_ROOT_PASS
    if (auth("root", timeout_p))
      break;
    else
      sleep(1);
#endif
  }

  /* restore the terminal */
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term_bak);

  exit (0);
}

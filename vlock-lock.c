/* vlock-lock.c -- locking routine for vlock,
 *                   the VT locking program for linux
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

#include "vlock.h"

int main(void) {
  char user[40];
  char *vlock_message;
  struct passwd *pw;
  struct termios term, term_bak;
  int restore_term = 0;

  /* ignore some signals */
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

  /* get the password entry */
  if ((pw = getpwuid(getuid())) == NULL) {
    perror("vlock-lock: getpwuid() failed");
    exit (111);
  }

  /* copy the username */
  strncpy(user, pw->pw_name, sizeof user - 1);
  user[sizeof user - 1] = '\0';

  /* get the vlock message from the environment */
  vlock_message = getenv("VLOCK_MESSAGE");

  /* disable terminal echoing and signals */
  if (tcgetattr(STDIN_FILENO, &term) == 0) {
    term_bak = term;
    term.c_lflag &= ~(ECHO|ISIG);
    restore_term = (tcsetattr(STDIN_FILENO, TCSANOW, &term) == 0);
  }

  for (;;) {
    /* clear the screen */
    fprintf(stderr, CLEAR_SCREEN);

    if (vlock_message)
      /* print vlock message */
      fprintf(stderr, "%s\n", vlock_message);

    /* wait for enter to be pressed */
    fprintf(stderr, "Please press [ENTER] to unlock.\n");
    while (fgetc(stdin) != '\n');

    if (auth(user))
      break;
    else
      sleep(1);

#ifndef NO_ROOT_PASS
    if (auth("root"))
      break;
    else
      sleep(1);
#endif
  }

  /* restore the terminal */
  if (restore_term)
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term_bak);

  exit (0);
}

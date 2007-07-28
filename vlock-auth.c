/* vlock-auth.c -- authentification routine for vlock,
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

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#define CLEAR_SCREEN "\033[H\033[J"

int auth(const char *user);

int main(void) {
  char user[40];
  char *vlock_message;
  struct passwd *pw;

  /* ignore some signals */
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

  /* get the password entry */
  if ((pw = getpwuid(getuid())) == NULL) {
    perror("vlock: getpwuid() failed");
    exit (111);
  }

  /* copy the username */
  strncpy(user, pw->pw_name, sizeof user - 1);
  user[sizeof user - 1] = '\0';

  /* get the vlock message from the environment */
  vlock_message = getenv("VLOCK_MESSAGE");

  for (;;) {
    /* clear the screen */
    fprintf(stderr, CLEAR_SCREEN);

    if (vlock_message)
      /* print vlock message */
      fprintf(stderr, "%s\n", vlock_message);

    fflush(stderr);

    if (auth(user))
      exit (0);
    else
      sleep(1);

#ifndef NO_ROOT_PASS
    if (auth("root"))
      exit (0);
    else
      sleep(1);
#endif
  }
}

/* auth-shadow.c -- shadow authentification routine for vlock,
 *                  the VT locking program for linux
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

/* for crypt() */
#define _XOPEN_SOURCE

#ifndef __FreeBSD__
/* for asprintf() */
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <sys/mman.h>

#include <shadow.h>

#include "vlock.h"

int auth(const char *user, const struct timespec *timeout) {
  char *pwd;
  char *cryptpw;
  char *msg = NULL;
  struct spwd *spw;
  int result = 0;

  /* format the prompt */
  (void) asprintf(&msg, "%s's Password: ", user);

  if ((pwd = prompt_echo_off(msg, timeout)) == NULL)
    goto out;

  /* get the shadow password */
  if ((spw = getspnam(user)) == NULL)
    goto out_shadow;

  /* hash the password */
  if ((cryptpw = crypt(pwd, spw->sp_pwdp)) == NULL) {
    perror("vlock-auth: crypt()");
    goto out_shadow;
  }

  result = (strcmp(cryptpw, spw->sp_pwdp) == 0);

  if (!result) {
    sleep(1);
    fprintf(stderr, "vlock-auth: Authentication error\n");
  }

out_shadow:
  /* deallocate shadow resources */
  endspent();

out:
  /* free the prompt */
  free(msg);

  /* free the password */
  free(pwd);

  return result;
}

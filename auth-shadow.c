/* auth-shadow.c -- shadow authentification routine for vlock,
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

#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>

#include <shadow.h>

#define PWD_BUFFER_SIZE 256

/* Try to authenticate the user.  When the user is successfully authenticated
 * this function returns 1.  When the authentication fails for whatever reason
 * the function returns 0.
 */
int auth(const char *user) {
  char buffer[PWD_BUFFER_SIZE];
  size_t pwlen;
  char *cryptpw;
  struct spwd *spw;
  int result = 0;

  /* lock the password buffer */
  (void) mlock(buffer, sizeof buffer);
  
  /* write out the prompt */
  fprintf(stderr, "%s's Password: ", user); fflush(stderr);

  /* read the password */
  if (fgets(buffer, sizeof buffer, stdin) == NULL)
    goto out;

  /* put newline */
  fputc('\n', stderr);

  pwlen = strlen(buffer);

  /* strip the newline */
  if (buffer[pwlen-1] == '\n')
    buffer[pwlen-1] = '\0';

  /* get the shadow password */
  if ((spw = getspnam(user)) == NULL)
    goto out_shadow;

  /* hash the password */
  if ((cryptpw = crypt(buffer, spw->sp_pwdp)) == NULL) {
    perror("vlock: crypt()");
    goto out_shadow;
  }

  /* XXX: sp_lstchg, sp_min, sp_inact, sp_expire should also be checked here */

  result = strcmp(cryptpw, spw->sp_pwdp) == 0;

out_shadow:
  /* deallocate shadow resources */
  endspent();

out:
  /* clear the buffer */
  memset(buffer, 0, sizeof buffer);

  /* unlock the password buffer */
  (void) munlock(buffer, sizeof buffer);

  return result;
}

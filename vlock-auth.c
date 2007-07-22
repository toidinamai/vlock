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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

int auth(const char *user) {
  pam_handle_t *pamh;
  int pam_status;
  int auth_status;
  struct pam_conv pamc = {
    &misc_conv,
    NULL
  };

  pam_status = pam_start("vlock", user, &pamc, &pamh);

  if (pam_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_status));
    auth_status = -1;
    goto out;
  }

  fprintf(stderr, "%s's ", user); fflush(stdout);
  pam_status = pam_authenticate(pamh, 0);

  if (pam_status == PAM_SUCCESS) {
    auth_status = 1;
  } else if (pam_status == PAM_AUTH_ERR) {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_status));
    auth_status = 0;
  } else {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_status));
    auth_status = -1;
  }

out:
  pam_status = pam_end(pamh, pam_status);

  if (pam_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_status));
  }

  if (auth_status < 0)
    exit (111);
  else
    return auth_status;
}

int main(void) {
  char user[40];
  struct passwd *pw = getpwuid(getuid());

  if (pw == NULL) {
    perror("vlock: getpwuid() failed");
    exit (111);
  }

  strncpy(user, pw->pw_name, sizeof user - 1);
  user[sizeof user - 1] = '\0';

  if (auth(user) != 0)
    exit (0);

#ifndef NO_ROOT_PASS
  if (auth("root") != 0)
    exit (0);
#endif

  exit (1);
}

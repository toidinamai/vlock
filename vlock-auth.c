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
#include <signal.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#define CLEAR_SCREEN "\033[H\033[J"

/* Try to authenticate the user.  When the user is successfully authenticated
 * this function exits the program with exit status 0.  When the authentication
 * failed because of bad user input this function returns 1.  For all other
 * errors -1 is returned. */
int auth_exit(const char *user) {
  pam_handle_t *pamh;
  int pam_status;
  int pam_end_status;
  struct pam_conv pamc = {
    &misc_conv,
    NULL
  };

  /* initialize pam */
  pam_status = pam_start("vlock", user, &pamc, &pamh);

  if (pam_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_status));
    goto end;
  }

  /* put the username before the password prompt */
  fprintf(stderr, "%s's ", user); fflush(stderr);
  /* authenticate the user */
  pam_status = pam_authenticate(pamh, 0);

  if (pam_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_status));
  }

end:
  /* finish pam */
  pam_end_status = pam_end(pamh, pam_status);

  if (pam_end_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock: %s\n", pam_strerror(pamh, pam_end_status));
  }

  switch (pam_status) {
    case PAM_SUCCESS:
      exit (0);
    case PAM_USER_UNKNOWN:
    case PAM_AUTH_ERR:
      return 1;
    default:
      return -1;
  }
}

int main(void) {
  char user[40];
  char *vlock_message;
  struct passwd *pw;

  /* ignore some signals */
  signal(SIGINT, SIG_IGN);
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

    if (auth_exit(user) < 0)
      sleep(1);

#ifndef NO_ROOT_PASS
    if (auth_exit("root") < 0)
      sleep(1);
#endif
  }
}

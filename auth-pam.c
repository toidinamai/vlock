/* auth-pam.c -- PAM authentification routine for vlock,
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

#include <stdio.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

int auth_service(const char *user, const char *service) {
  pam_handle_t *pamh;
  int pam_status;
  int pam_end_status;
  struct pam_conv pamc = {
    &misc_conv,
    NULL
  };

  /* initialize pam */
  pam_status = pam_start(service, user, &pamc, &pamh);

  if (pam_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock-auth: %s\n", pam_strerror(pamh, pam_status));
    goto end;
  }

  /* put the username before the password prompt */
  fprintf(stderr, "%s's ", user); fflush(stderr);
  /* authenticate the user */
  pam_status = pam_authenticate(pamh, 0);

  if (pam_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock-auth: %s\n", pam_strerror(pamh, pam_status));
  }

end:
  /* finish pam */
  pam_end_status = pam_end(pamh, pam_status);

  if (pam_end_status != PAM_SUCCESS) {
    fprintf(stderr, "vlock-auth: %s\n", pam_strerror(pamh, pam_end_status));
  }

  return (pam_status == PAM_SUCCESS);
}

int auth(const char *user) {
  return auth_service(user, "vlock");
}

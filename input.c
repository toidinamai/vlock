/* input.c -- password getting module for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

  /* Marek hacked the code a bit:
      - use /dev/tty instead of /dev/console (permissions on the latter
        shouldn't allow all users to open it)
      - get encrypted password once at startup, drop privileges as soon
        as possible (we have to be setuid root for shadow passwords)
        * This may not be a good idea, because it makes vlock a way for
          the user to get at root's encrypted password (if user can
          ptrace process after setuid(getuid()); -- check this)
        * (probably) can't give up id with PAM anyway, at least with
          shadow
      - error message instead of core dump if getpwuid() returns NULL
      - added check if encrypted password is valid (>=13 characters)
      - terminal modes should be properly restored (don't leave echo off)
        if getpass() fails.
  */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

static struct pam_conv PAM_conversation = {
    &misc_conv,
    NULL
};

#include "vlock.h"


static char prompt[100];  /* password prompt ("user's password: ") */
static char username[40]; /* current user's name */


static int
correct_password(void)
{
  pam_handle_t *pamh;
  int pam_error;

  /* Now use PAM to do authentication.
   */
  #define PAM_BAIL if (pam_error != PAM_SUCCESS) { \
     pam_end(pamh, 0); \
     /* fix signals that may have been disordered by pam */ \
     set_signal_mask(0); \
     return 0; \
     }
  pam_error = pam_start("vlock", username, &PAM_conversation, &pamh);
  PAM_BAIL;
  printf("%s's ", username); fflush(stdout);
  pam_error = pam_set_item(pamh, PAM_USER_PROMPT, strdup(prompt));
  PAM_BAIL;
  pam_error = pam_authenticate(pamh, 0);
  /* fix signals that may have been disordered by pam */
  set_signal_mask(0);
#ifdef NO_ROOT_PASS
  PAM_BAIL;
#else
  if (pam_error != PAM_SUCCESS) {
    /* Try as root; bail if no success there either */
    printf("root's "); fflush(stdout);
    pam_error = pam_set_item(pamh, PAM_USER_PROMPT, strdup(prompt));
    PAM_BAIL;
    pam_error = pam_set_item(pamh, PAM_USER, "root");
    PAM_BAIL;
    pam_error = pam_authenticate(pamh, 0);
    /* fix signals that may have been disordered by pam */
    set_signal_mask(0);
    PAM_BAIL;
  }
#endif /* !NO_ROOT_PASS */
  pam_end(pamh, PAM_SUCCESS);
  /* If this point is reached, the user has been authenticated. */
  return 1;
}




void
get_password(void)
{
  set_terminal(0);
  do {
    if (o_lock_all) {
      /* To do: allow logging the user out safely without shutting down
         the whole machine...  */
      printf("The entire console display is now completely locked.\n"
       "You will not be able to switch to another virtual console.\n");

    } else {
      printf("This TTY is now locked.\n");
      if (is_vt)
        printf("Use Alt-function keys to switch to other virtual consoles.\n");
    }
    printf("Please enter the password to unlock.\n");
    fflush(stdout);

    /* correct_password() sets the terminal status as necessary */
    if (correct_password()) {
      restore_signals();
      restore_terminal();
      return;
    }

    /* correct_password() may have failed to return success because the   */
    /* terminal is closed.  In this case, it would not be appropriate to  */
    /* try again...                                                       */
    if (isatty(STDIN_FILENO) == 0) {
	perror("isatty");
	restore_terminal();
	exit(1);
    }


    /* Need to slow down people who are trying to break in by brute force */
    /* Note that it is technically possible to break this, but I can't    */
    /* make it happen, even knowing the code and knowing how to try.      */
    /* If you manage to kill vlock from the terminal while in this code,  */
    /* please tell me how you did it.                                     */
    printf(" *** That password is incorrect; please try again. *** \n");
    printf("\n");

  } while (1);
}

  /* Get the user's and root passwords once at startup and store them
     for use later.  This has several advantages:
      - to support shadow passwords, we have to be installed setuid
        root, but we can completely drop privileges very early
      - we avoid problems with unlocking when using NIS, after the
        NIS server goes down
      - before locking, we can check if any real password has a chance
        to match the encrypted password (should be >=13 chars)
      - this is the same way xlockmore does it.
     Warning: this code runs as root - be careful if you modify it.

     With PAM, applications never see encrypted passwords, so we can't
     get the encrypted password ahead of time.  We just have to hope
     things work later on, just like we do with PAMified xlockmore.
   */

static struct passwd *
my_getpwuid(uid_t uid)
{
  struct passwd *pw;

  pw = getpwuid(uid);
  if (!pw) {
    fprintf(stderr, "vlock: getpwuid(%d) failed: %s\n", uid, strerror(errno));
    exit(1);
  }
  return pw;
}

void
init_passwords(void)
{
  struct passwd *pw;

  /* Get the password entry for this user (never returns NULL).  */
  pw = my_getpwuid(getuid());

  /* Save the results where they will not get overwritten.  */
  strncpy(username, pw->pw_name, sizeof(username) - 1);
  username[sizeof(username) - 1] = '\0';
}

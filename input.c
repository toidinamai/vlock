/* input.c -- password getting module for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#ifdef SHADOW_PWD
  /* Shadow passwd support; THIS IS NOT SAFE with some very old versions
     of the shadow password suite, where some signals get ignored, and
     simple keystrokes can terminate vlock.  This cannot be solved here;
     you have to fix your shadow library or use a working one. */

  /* Current shadow maintainer's note: it should no longer be a problem
     with libc5 because we don't need to link with libshadow.a at all
     (shadow passwords are now supported in the standard libc). */


#include <shadow.h>
#endif


#ifdef USE_PAM
  /* PAM -- Pluggable Authentication Modules support
     With any luck, PAM will be *the* method through which authentication
     is done under Linux.  Very flexible; allows any authentication method
     to work according to a configuration file.
  */
  /* PAM subsumes shadow, and doesn't mesh with shadow-specific code */
#ifdef SHADOW_PWD
#error "Shadow and PAM don't mix!"
#endif

#include <security/pam_appl.h>
/* Static variables used to communicate between the conversation function
 * and the server_login function
 */
static char *PAM_username;
static char *PAM_password;

/* PAM conversation function
 * Here we assume (for now, at least) that echo on means login name, and
 * echo off means password.
 * FIXME: move to misc_conv and do things the right way!
 */
static int PAM_conv (int num_msg,
                     const struct pam_message **msg,
		     struct pam_response **resp,
		     void *appdata_ptr) {
  int count = 0, replies = 0;
  struct pam_response *reply = NULL;
  int size = sizeof(struct pam_response);

  reply = malloc(sizeof(struct pam_response) * num_msg);
  if (!reply) {
    return PAM_CONV_ERR;
  }

  #define COPY_STRING(s) (s) ? strdup(s) : NULL

  for (count = 0; count < num_msg; count++, replies++) {
    switch (msg[count]->msg_style) {
      case PAM_PROMPT_ECHO_ON:
        reply[replies].resp_retcode = PAM_SUCCESS;
	reply[replies].resp = COPY_STRING(PAM_username);
          /* PAM frees resp */
        break;
      case PAM_PROMPT_ECHO_OFF:
        reply[replies].resp_retcode = PAM_SUCCESS;
	reply[replies].resp = COPY_STRING(PAM_password);
          /* PAM frees resp */
        break;
      case PAM_TEXT_INFO:
        reply[replies].resp_retcode = PAM_SUCCESS;
	reply[replies].resp = NULL;
        /* ignore it... */
        break;
      case PAM_ERROR_MSG:
        reply[replies].resp_retcode = PAM_SUCCESS;
	reply[replies].resp = NULL;
	break;
      default:
        /* Must be an error of some sort... */
        free (reply);
        return PAM_CONV_ERR;
    }
  }
  *resp = reply;
  return PAM_SUCCESS;
}
static struct pam_conv PAM_conversation = {
    &PAM_conv,
    NULL
};

#endif /* USE_PAM */
#include "vlock.h"


static char rcsid[] = "$Id: input.c,v 1.17 1997/10/10 17:10:27 johnsonm Exp $";


static char username[40]; /* current user's name */
static char prompt[100];  /* password prompt ("user's password: ") */
#ifndef USE_PAM
static char userpw[200];  /* current user's encrypted password */
#ifndef NO_ROOT_PASS
static char rootpw[200];  /* root's encrypted password */
#endif
#endif /* !USE_PAM */



static int
correct_password(void)
{
  char *pass;
#ifdef USE_PAM
  pam_handle_t *pamh;
  int pam_error;
#endif

  pass = getpass(prompt);
  if (!pass) {
    perror("getpass: cannot open /dev/tty");
    restore_terminal();
    exit(1);
  }
  /* fix signals that probably have been disordered by getpass() */
  set_signal_mask(0);

#ifdef USE_PAM
  /* Now use PAM to do authentication.
   */
  #define PAM_BAIL if (pam_error != PAM_SUCCESS) { \
     pam_end(pamh, 0); \
     memset(pass, 0, strlen(pass)); \
     return 0; \
     }
  PAM_password = pass;
  PAM_username = username;
  pam_error = pam_start("vlock", username, &PAM_conversation, &pamh);
  PAM_BAIL;
  pam_error = pam_authenticate(pamh, 0);
#ifdef NO_ROOT_PASS
  PAM_BAIL;
#else
  if (pam_error != PAM_SUCCESS) {
    /* Try as root; bail if no success there either */
    pam_error = pam_set_item(pamh, PAM_USER, "root");
    PAM_BAIL;
    pam_error = pam_authenticate(pamh, 0);
    PAM_BAIL;
  }
#endif /* !NO_ROOT_PASS */
  pam_end(pamh, PAM_SUCCESS);
  /* If this point is reached, the user has been authenticated. */
  memset(pass, 0, strlen(pass));
  return 1;

#else /* !PAM */
  if (strcmp(crypt(pass, userpw), userpw) == 0) {
    memset(pass, 0, strlen(pass));
    return 1;
  }
#ifndef NO_ROOT_PASS
  if (strcmp(crypt(pass, rootpw), rootpw) == 0) {
    memset(pass, 0, strlen(pass));
    /* To do: for the paranoid, maybe log the use of root password? */
    return 1;
  }
#endif
#endif /* !USE_PAM */

  memset(pass, 0, strlen(pass));
  return 0;
}




void
get_password(void)
{
  int times = 0;

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
    /* Need to slow down people who are trying to break in by brute force */
    /* Note that it is technically possible to break this, but I can't    */
    /* make it happen, even knowing the code and knowing how to try.      */
    /* If you manage to kill vlock from the terminal while in this code,  */
    /* please tell me how you did it.                                     */
    sleep(++times);
    printf(" *** That password is incorrect; please try again. *** \n");
    if (times >= 15) {
      printf("Slow down and try again in a while.\n");
      sleep(times);
      times = 2; /* don't make things too easy for someone to break in */
    }
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
#ifdef SHADOW_PWD
  struct spwd *sp;
#endif

  pw = getpwuid(uid);
  if (!pw) {
    fprintf(stderr, "vlock: getpwuid(%d) failed!\n", (int) uid);
    exit(1);
  }
#ifdef SHADOW_PWD
  sp = getspnam(pw->pw_name);
  if (sp)
    pw->pw_passwd = sp->sp_pwdp;
  endspent();
#endif
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

#ifndef USE_PAM
  strncpy(userpw, pw->pw_passwd, sizeof(userpw) - 1);
  userpw[sizeof(userpw) - 1] = '\0';

  if (strlen(userpw) < 13) {
    /* To do: ask for password to use instead of login password.  */
    fprintf(stderr,
      " *** No valid password for user %s - will not lock.  ***\n", username);
    exit(1);
  }

#ifndef NO_ROOT_PASS
  /* Now get and save the encrypted root password.  */
  pw = my_getpwuid(0);

  strncpy(rootpw, pw->pw_passwd, sizeof(rootpw) - 1);
  rootpw[sizeof(rootpw) - 1] = '\0';
#endif /* !NO_ROOT_PASS */

  /* We don't need root privileges any longer.  */
  setuid(getuid());
#endif /* !USE_PAM */

  sprintf(prompt, "%s's password: ", username);
}


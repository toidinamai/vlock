/* input.c -- password getting module for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

/* RCS log:
 * $Log: input.c,v $
 * Revision 1.13  1994/07/03 13:07:47  johnsonm
 * Set and restore signals and terminal settings correctly.
 *
 * Revision 1.12  1994/07/03  12:29:27  johnsonm
 * Don't restore terminal, which restores signals, which makes vlock -a
 *   completely ineffective, until the correct password has been entered.
 *
 * Revision 1.11  1994/07/03  12:19:29  johnsonm
 * Moved shadow comment to first SHADOW define.
 *
 * Revision 1.10  1994/07/03  12:14:49  johnsonm
 * Restores signal mask after getting password
 *
 * Revision 1.9  1994/07/03  12:11:24  johnsonm
 * Updated shadow comment to explain.
 *
 * Revision 1.8  1994/03/23  17:00:01  johnsonm
 * Control-function -> Alt-function
 * Added support for non-vt consoles.
 *
 * Revision 1.7  1994/03/21  17:33:33  johnsonm
 * Reformatted "This TTY is now locked" message to read easier.
 *
 * Revision 1.6  1994/03/21  17:30:55  johnsonm
 * Allow root to lock console.  Was only disallowed because I took
 * the correct_password function from gnu su, where of course root
 * automatically succeeds at entering the password without haveing
 * to enter it.  Oops.
 *
 * Revision 1.5  1994/03/20  13:13:48  johnsonm
 * Added code so that root password unlocks the session too.
 *
 * Revision 1.4  1994/03/19  17:53:20  johnsonm
 * Password reading now works, thanks to the correct_password() routine
 * from GNU su.  No point in re-inventing the wheel...
 *
 * Revision 1.3  1994/03/19  14:24:33  johnsonm
 * Removed silly two-process model.  It's certainly not needed.
 * Also fixed occasional core dump.
 *
 * Revision 1.2  1994/03/16  20:12:06  johnsonm
 * Now almost working.  Need to get signals straightened out for
 * second process.
 *
 * Revision 1.1  1994/03/15  18:27:33  johnsonm
 * Initial revision
 *
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#ifdef SHADOW_PWD
  /* Shadow passwd support; THIS IS NOT SAFE with some shadow password
     versions, where some signals get ignored, and simple keystrokes
     can terminate vlock.  This cannot be solved here; you have to
     fix your shadow library or use a working one. */

  /* Current shadow maintainer's note: it should no longer be a problem
     with libc5 because we don't need to link with libshadow.a at all
     (shadow passwords are now supported in the standard libc).

     I also hacked the code a bit:
      - use /dev/tty instead of /dev/console (permissions on the latter
        shouldn't allow all users to open it)
      - get encrypted password once at startup, drop privileges as soon
        as possible (we have to be setuid root for shadow passwords)
      - error message instead of core dump if getpwuid() returns NULL
      - added check if encrypted password is valid (>=13 characters)
      - terminal modes should be properly restored (don't leave echo off)
        if getpass() fails.

     -- Marek Michalkiewicz <marekm@i17linuxb.ists.pwr.wroc.pl> */

#include <shadow.h>
#endif
#include "vlock.h"


static char rcsid[] = "$Id: input.c,v 1.14 1996/05/17 02:49:46 johnsonm Exp $";


static char username[40]; /* current user's name */
static char prompt[100];  /* password prompt ("user's password: ") */
static char userpw[200];  /* current user's encrypted password */
#ifndef NO_ROOT_PASS
static char rootpw[200];  /* root's encrypted password */
#endif
static int
correct_password(void)
{
  char *pass;

  pass = getpass(prompt);
  if (!pass) {
    perror("getpass: cannot open /dev/tty");
    restore_terminal();
    exit(1);
  }
  /* fix signals that probably have been disordered by getpass() */
  set_signal_mask(0);

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
     Warning: this code runs as root - be careful if you modify it.  */

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
  strncpy(userpw, pw->pw_passwd, sizeof(userpw) - 1);
  userpw[sizeof(userpw) - 1] = '\0';

  strncpy(username, pw->pw_name, sizeof(username) - 1);
  username[sizeof(username) - 1] = '\0';

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
#endif /* NO_ROOT_PASS */

  /* We don't need root privileges any longer.  */
  setuid(getuid());

  sprintf(prompt, "%s's password: ", username);
}

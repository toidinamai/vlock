/* input.c -- password getting module for vlock
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software, which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

/* RCS log:
 * $Log: input.c,v $
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
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#ifdef SHADOW_PWD
#include <shadow.h>
#endif
#include "vlock.h"


/* size of input buffer. */
#define INBUFSIZE 50


static char rcsid[] = "$Id: input.c,v 1.8 1994/03/23 17:00:01 johnsonm Exp $";


/* correct_password() taken with some modifications from the GNU su.c */
/* returns true if the password entered is either the user's password */
/* or the root password, so that root can unlock anything...          */

static int correct_password (struct passwd *pw) {

  char *runencrypted, *unencrypted, *encrypted, *correct, user[INBUFSIZE];
  int ret;
  static struct passwd rpw;

#ifdef SHADOW_PWD
  /* Shadow passwd support; untested.  */
  struct spwd *sp = getspnam(pw->pw_name);

  endspent ();
  if (sp)
    correct = sp->sp_pwdp;
  else
#endif
  correct = pw->pw_passwd;

  /* if account has no password, it can't be locked.  I'm not */
  /* going to fake it.  The user can write a shell script to  */
  /* fake it if he wants to...                                */
  if (correct == 0 || correct[0] == '\0')
    return 1;

  sprintf(user, "%s's password:", pw->pw_name);
  unencrypted = getpass(user);
  runencrypted = (char *) strdup(unencrypted);
  if (unencrypted == NULL) {
    perror ("getpass: cannot open /dev/tty");
    /* Need to exit, or computer might be locked up... */
    exit (1);
  }
  encrypted = crypt(unencrypted, correct);
  ret = (strcmp(encrypted, correct) == 0);
  if (!ret) {
#ifdef SHADOW_PWD
    struct spwd *rsp = getspnam("root");
#endif
    rpw = *(getpwuid(0)); /* get the root password */

#ifdef SHADOW_PWD
    endspent ();
    if (rsp)
      correct = rsp->sp_pwdp;
    else
#endif
    correct = rpw.pw_passwd;
    encrypted = crypt(runencrypted, correct);
    ret = (strcmp(encrypted, correct) == 0);
  }
  memset (unencrypted, 0, strlen(unencrypted));
  memset (runencrypted, 0, strlen(runencrypted));
  free(runencrypted);
  return ret;
}



/* get_password() should not return until a correct password has been */
/* entered. */

void get_password(void) {

  static struct passwd pwd;
  int times = 0;

  do {
    pwd = *(getpwuid(getuid()));
    if (o_lock_all) {
      printf("The entire console display is now completely locked.\n"
	     "You will not be able to switch to another virtual console.\n");

    } else {
      printf("This TTY is now locked.\n");
      if (is_vt)
	printf("Use Alt-function keys to switch to other virtual consoles.\n");
    }
    printf("Please enter the password to unlock.\n");
    fflush(stdout);

    if (correct_password(&pwd))
      return;

    /* Need to slow down people who are trying to break in by brute force */
    /* Note that it is technically possible to break this, but I can't    */
    /* make it happen, even knowing the code and knowing how to try.      */
    /* If you manage to kill vlock from the terminal while in this code,  */
    /* please tell me how you did it.                                     */
    set_terminal();
    sleep(++times);
    printf(" *** That password is incorrect; please try again. *** \n");
    if (times >= 15) {
      printf("Slow down and try again in a while.\n");
      sleep(times);
      times = 2; /* don't make things too easy for someone to break in */
    }
    printf("\n");
    restore_terminal();

  } while (1);

}

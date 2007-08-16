/* vlock-current.c -- locking routine for vlock,
 *                    the VT locking program for linux
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

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/select.h>

#include "vlock.h"

/* Read a single character from the stdin.  If the timeout is reached
 * 0 is returned. */
char read_character(struct timespec *timeout) {
  char c = 0;
  struct timeval *timeout_val = NULL;
  fd_set readfds;

  if (timeout != NULL) {
    timeout_val = calloc(sizeof *timeout_val, 1);

    if (timeout_val != NULL) {
      timeout_val->tv_sec = timeout->tv_sec;
      timeout_val->tv_usec = timeout->tv_nsec / 1000;
    }
  }

  /* Initialize file descriptor set. */
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  /* Wait for a character. */
  if (select(STDIN_FILENO+1, &readfds, NULL, NULL, timeout_val) != 1)
    goto out;

  /* Read the character. */
  (void) read(STDIN_FILENO, &c, 1);

out:
  free(timeout_val);
  return c;
}

/* Parse the given string (interpreted as seconds) into a
 * timespec.  On error NULL is returned.  The caller is responsible
 * to free the result.   The string may be NULL, in which case NULL
 * is returned, too. */
struct timespec *parse_seconds(const char *s) {
  if (s == NULL)
    return NULL;
  else {
    char *n;
    struct timespec *t = calloc(sizeof *t, 1);

    if (t == NULL)
      return NULL;

    t->tv_sec = strtol(s, &n, 10);

    if (*n != '\0' || t->tv_sec <= 0) {
      free(t);
      t = NULL;
    }

    return t;
  }
}

/* Lock the current terminal until proper authentication is received. */
int main(void) {
  char user[40];
  char *vlock_message;
  struct timespec *prompt_timeout;
  struct timespec *timeout;
  struct termios term;
  tcflag_t lflag;
  struct sigaction sa;
  /* get the user id */
  uid_t uid = getuid();
  /* get the user name from the environment */
  char *envuser = getenv("USER"); 

  /* ignore some signals */
  /* these signals shouldn't be delivered anyway, because
   * terminal signals are disabled below */
  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = SIG_IGN;
  (void) sigaction(SIGINT, &sa, NULL);
  (void) sigaction(SIGQUIT, &sa, NULL);
  (void) sigaction(SIGTSTP, &sa, NULL);

  if (uid > 0 || envuser == NULL) {
    errno = 0;

    /* get the password entry */
    struct passwd *pw = getpwuid(uid);

    if (pw == NULL) {
      if (errno != 0)
        perror("vlock-current: getpwuid() failed");
      else
        fprintf(stderr, "vlock-current: getpwuid() failed\n");

      exit (111);
    }

    /* copy the username */
    strncpy(user, pw->pw_name, sizeof user - 1);
    user[sizeof user - 1] = '\0';
  } else {
    /* copy the username */
    strncpy(user, envuser, sizeof user - 1);
    user[sizeof user - 1] = '\0';
  }

#ifdef USE_PLUGINS
  load_plugins();
#endif

  /* get the vlock message from the environment */
  vlock_message = getenv("VLOCK_MESSAGE");

  /* get the timeouts from the environment */
  prompt_timeout = parse_seconds(getenv("VLOCK_PROMPT_TIMEOUT"));
#ifdef USE_PLUGINS
  timeout = parse_seconds(getenv("VLOCK_TIMEOUT"));
#else
  timeout = NULL;
#endif

  /* disable terminal echoing and signals */
  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~(ECHO|ISIG);
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

#ifdef USE_PLUGINS
  plugin_hook("vlock_start");
#endif

  for (;;) {
    tcflag_t lflag = term.c_lflag;;

    if (vlock_message) {
      /* print vlock message */
      fputs(vlock_message, stderr);
      fputc('\n', stderr);
    }

    /* switch of line buffering */
    term.c_lflag &= ~ICANON;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

    /* wait for enter to be pressed */
    for (;;) {
      char c = read_character(timeout);

      if (c == '\n') {
        break;
      }
#ifdef USE_PLUGINS
      else if (c == '\033' || c == 0) {
        plugin_hook("vlock_save");
        (void) read_character(NULL);
        plugin_hook("vlock_save_abort");
      }
#endif
    }

    /* restore line buffering */
    term.c_lflag = lflag;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

    if (auth(user, prompt_timeout))
      break;
    else
      sleep(1);

#ifndef NO_ROOT_PASS
    if (auth("root", prompt_timeout))
      break;
    else
      sleep(1);
#endif
  }

#ifdef USE_PLUGINS
  plugin_hook("vlock_end");
#endif

  /* restore the terminal */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

  /* free some memory */
  free(timeout);
  free(prompt_timeout);

  exit (0);
}

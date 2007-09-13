/* vlock-main.c -- main routine for vlock,
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

#include "prompt.h"
#include "auth.h"

#ifdef USE_PLUGINS
#include "plugins.h"
#endif

static struct termios term;

/* Wait for any of the characters in the given character set to be read from
 * stdin.  If charset is NULL wait for any character.  Returns 0 when the
 * timeout occurs. */
static char wait_for_character(const char *charset, struct timespec *timeout) {
  tcflag_t lflag;
  char c;

  /* switch off line buffering */
  lflag = term.c_lflag;
  term.c_lflag &= ~ICANON;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

  for (;;) {
    c = read_character(timeout);

    if (c == 0 || charset == NULL)
      break;
    else if (strchr(charset, c) != NULL)
      break;
  }

  /* restore line buffering */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

  return c;
}

/* Parse the given string (interpreted as seconds) into a
 * timespec.  On error NULL is returned.  The caller is responsible
 * to free the result.   The string may be NULL, in which case NULL
 * is returned, too. */
static struct timespec *parse_seconds(const char *s)
{
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
int main(int argc, char *const argv[])
{
  int exit_status = 0;
  char user[40];
  char *vlock_message;
  struct timespec *prompt_timeout;
  struct timespec *timeout;
  tcflag_t lflag;
  struct sigaction sa;
  /* get the user id */
  uid_t uid = getuid();
#ifdef USE_PLUGINS
  /* get the effective user id */
  uid_t euid = geteuid();
#endif
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
    struct passwd *pw;

    /* clear errno */
    errno = 0;

    /* get the password entry */
    pw = getpwuid(uid);

    if (pw == NULL) {
      if (errno != 0)
        perror("vlock-main: getpwuid() failed");
      else
        fprintf(stderr, "vlock-main: getpwuid() failed\n");

      exit(111);
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
  /* drop privileges while loading plugins */
  seteuid(getuid());

  for (int i = 1; i < argc; i++) {
    errno = 0;

    if (!load_plugin(argv[i])) {
      if (errno)
        fprintf(stderr, "vlock-main: error loading plugin '%s': %s\n",
                argv[i], strerror(errno));
      else
        fprintf(stderr, "vlock-main: error loading plugin '%s'\n", argv[i]);

      exit(111);
    }
  }

  /* regain privileges after loading plugins */
  seteuid(euid);

  if (!resolve_dependencies()) {
    fprintf(stderr, "vlock-main: plugin dependency error\n");
    exit(111);
  }
#endif

  /* get the vlock message from the environment */
  vlock_message = getenv("VLOCK_MESSAGE");

#ifdef USE_PLUGINS
  if (vlock_message == NULL) {
    if (is_loaded("all"))
      vlock_message = getenv("VLOCK_ALL_MESSAGE");
    else
      vlock_message = getenv("VLOCK_CURRENT_MESSAGE");
  }
#endif

  /* get the timeouts from the environment */
  prompt_timeout = parse_seconds(getenv("VLOCK_PROMPT_TIMEOUT"));
#ifdef USE_PLUGINS
  timeout = parse_seconds(getenv("VLOCK_TIMEOUT"));
#else
  timeout = NULL;
#endif

#ifdef USE_PLUGINS
  if (!plugin_hook("vlock_start")) {
    fprintf(stderr, "vlock-main: error in 'vlock_start' hook\n");
    exit_status = 111;
    goto out;
  }
#else
  /* call vlock-new and vlock-all statically */
#error "Not implemented."
#endif

  /* disable terminal echoing and signals */
  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~(ECHO | ISIG);
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

  for (;;) {
    char c;

    if (vlock_message) {
      /* print vlock message */
      fputs(vlock_message, stderr);
      fputc('\n', stderr);
    }

    /* wait for enter or escape to be pressed */
    c = wait_for_character("\n\033", timeout);

    /* escape was pressed or the timeout occurred */
    if (c == '\033' || c == 0) {
#ifdef USE_PLUGINS
      if (plugin_hook("vlock_save"))
        /* wait for any key to be pressed */
        (void) wait_for_character(NULL, NULL);

      (void) plugin_hook("vlock_save_abort");
#endif
      continue;
    }

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

  /* restore the terminal */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

#ifdef USE_PLUGINS
out:
  (void) plugin_hook("vlock_end");
  unload_plugins();
#else
  /* call vlock-new and vlock-all statically */
#error "Not implemented."
#endif

  /* free some memory */
  free(timeout);
  free(prompt_timeout);

  exit(exit_status);
}

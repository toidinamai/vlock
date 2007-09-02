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

#include "vlock.h"

#ifdef USE_PLUGINS
#include "plugins.h"
#endif

/* Read a single character from the stdin.  If the timeout is reached
 * 0 is returned. */
static char read_character(struct timespec *timeout)
{
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
  if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, timeout_val) != 1)
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
  struct termios term;
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

    if (!load_plugin(argv[i], VLOCK_PLUGIN_DIR)) {
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

  if (!resolve_dependencies())
    exit(111);
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

  /* disable terminal echoing and signals */
  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~(ECHO | ISIG);
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

#ifdef USE_PLUGINS
  if (!plugin_hook("vlock_start")) {
    exit_status = 111;
    goto out;
  }
#else
  /* call vlock-new and vlock-all statically */
#error "Not implemented."
#endif

  for (;;) {
    tcflag_t lflag = term.c_lflag;
    char *c;

    if (vlock_message) {
      /* print vlock message */
      fputs(vlock_message, stderr);
      fputc('\n', stderr);
    }

    /* switch off line buffering */
    term.c_lflag &= ~ICANON;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

    /* wait for enter or escape to be pressed */
    while ((c = strchr("\n\033", read_character(timeout))) == NULL);

    /* restore line buffering */
    term.c_lflag = lflag;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

    /* escape was pressed or the timeout occurred */
    if (*c == '\033' || *c == 0) {
#ifdef USE_PLUGINS
      if (plugin_hook("vlock_save"))
        /* wait for key press */
        (void) read_character(NULL);

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

#ifdef USE_PLUGINS
out:
  (void) plugin_hook("vlock_end");
  unload_plugins();
#else
  /* call vlock-new and vlock-all statically */
#error "Not implemented."
#endif

  /* restore the terminal */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

  /* free some memory */
  free(timeout);
  free(prompt_timeout);

  exit(exit_status);
}

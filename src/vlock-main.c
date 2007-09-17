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

#include "prompt.h"
#include "auth.h"
#include "console_switch.h"
#include "util.h"

#ifdef USE_PLUGINS
#include "plugins.h"
#endif

static char *get_username(void)
{
  /* get the user id */
  uid_t uid = getuid();
  char *username = NULL;

  if (uid == 0)
    /* get the user name from the environment */
    username = getenv("USER");

  if (username == NULL) {
    struct passwd *pw;

    /* get the password entry */
    pw = getpwuid(uid);

    if (pw == NULL)
      fatal_error("vlock-main: could not get the user name");

    username = pw->pw_name;
  }

  return ensure_not_null(strdup(username), "could not copy string");
}

static void terminate(int signum)
{
  if (signum == SIGTERM)
    fprintf(stderr, "vlock-main: Terminated!\n");
  else if (signum == SIGABRT)
    fprintf(stderr, "vlock-main: Aborted!\n");

  exit(1);
}

static void block_signals(void)
{
  struct sigaction sa;

  /* ignore some signals */
  /* these signals shouldn't be delivered anyway, because
   * terminal signals are disabled below */
  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = SIG_IGN;
  (void) sigaction(SIGINT, &sa, NULL);
  (void) sigaction(SIGQUIT, &sa, NULL);
  (void) sigaction(SIGTSTP, &sa, NULL);

  sa.sa_flags = SA_RESETHAND;
  sa.sa_handler = terminate;
  (void) sigaction(SIGTERM, &sa, NULL);
  (void) sigaction(SIGABRT, &sa, NULL);
}

static struct termios term;
static tcflag_t lflag;

static void secure_terminal(void)
{
  /* disable terminal echoing and signals */
  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~(ECHO | ISIG);
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

static void restore_terminal(void)
{
  /* restore the terminal */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

static void auth_loop(const char *username)
{
  struct timespec *prompt_timeout;
  struct timespec *wait_timeout;
  char *vlock_message;

  /* get the vlock message from the environment */
  vlock_message = getenv("VLOCK_MESSAGE");

  if (vlock_message == NULL) {
    if (console_switch_locked)
      vlock_message = getenv("VLOCK_ALL_MESSAGE");
    else
      vlock_message = getenv("VLOCK_CURRENT_MESSAGE");
  }

  /* get the timeouts from the environment */
  prompt_timeout = parse_seconds(getenv("VLOCK_PROMPT_TIMEOUT"));
#ifdef USE_PLUGINS
  wait_timeout = parse_seconds(getenv("VLOCK_TIMEOUT"));
#else
  wait_timeout = NULL;
#endif

  for (;;) {
    char c;

    if (vlock_message) {
      /* print vlock message */
      fputs(vlock_message, stderr);
      fputc('\n', stderr);
    }

    /* wait for enter or escape to be pressed */
    c = wait_for_character("\n\033", wait_timeout);

    /* escape was pressed or the timeout occurred */
    if (c == '\033' || c == 0) {
#ifdef USE_PLUGINS
      plugin_hook("vlock_save");
      /* wait for any key to be pressed */
      (void) wait_for_character(NULL, NULL);
      plugin_hook("vlock_save_abort");
#endif
      continue;
    }

    if (auth(username, prompt_timeout))
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

  /* free some memory */
  free(wait_timeout);
  free(prompt_timeout);
}

#ifdef USE_PLUGINS
static void call_end_hook(void)
{
  plugin_hook("vlock_end");
}
#endif

/* Lock the current terminal until proper authentication is received. */
int main(int argc, char *const argv[])
{
  char *username;

  block_signals();

  username = get_username();

#ifdef USE_PLUGINS
  for (int i = 1; i < argc; i++)
    load_plugin(argv[i]);

  ensure_atexit(unload_plugins);
  resolve_dependencies();
  plugin_hook("vlock_start");
  ensure_atexit(call_end_hook);
#else /* !USE_PLUGINS */
  if (argc == 2 && (strcmp(argv[1], "all") == 0)) {
    char *error = NULL;

    if (!lock_console_switch(&error)) {
      if (error != NULL) {
        fprintf(stderr, "vlock-main: %s\n", error);
        free(error);
        abort();
      } else {
        fatal_error("vlock-main: could not disable console switching");
      }
    }

    ensure_atexit(unlock_console_switch);
  } else if (argc > 1) {
    fatal_error("vlock-main: plugin support disabled");
  }
#endif

  secure_terminal();
  ensure_atexit(restore_terminal);

  auth_loop(username);

  free(username);

  exit(0);
}

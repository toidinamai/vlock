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

#define _GNU_SOURCE
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

#include <glib.h>

#include "prompt.h"
#include "auth.h"
#include "console_switch.h"
#include "util.h"
#include "logging.h"

#ifdef USE_PLUGINS
#include "plugins.h"
#endif

static GList *atexit_functions;

static void invoke_atexit_functions(void)
{
  while (atexit_functions != NULL) {
    (*(void (**)())&atexit_functions->data)();
    atexit_functions = g_list_delete_link(atexit_functions,
        atexit_functions);
  }
}

static void ensure_atexit(void (*function)(void))
{
  if (atexit_functions == NULL)
    atexit(invoke_atexit_functions);

  atexit_functions = g_list_prepend(atexit_functions,
      *(void **)&function);
}

static void terminate(int signum)
{
  invoke_atexit_functions();
  fprintf(stderr, "vlock: Killed by signal %d (%s)!\n", signum, strsignal(signum));
  raise(signum);
}

static void install_signal_handlers(void)
{
  struct sigaction sa;

  /* Ignore some signals. */
  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = SIG_IGN;
  (void) sigaction(SIGTSTP, &sa, NULL);

  /* Handle termination signals.  None of these should be delivered in a normal
   * run of the program because terminal signals (INT, QUIT) are disabled
   * below. */
  sa.sa_flags = SA_RESETHAND;
  sa.sa_handler = terminate;
  (void) sigaction(SIGINT, &sa, NULL);
  (void) sigaction(SIGQUIT, &sa, NULL);
  (void) sigaction(SIGTERM, &sa, NULL);
  (void) sigaction(SIGHUP, &sa, NULL);
  (void) sigaction(SIGABRT, &sa, NULL);
  (void) sigaction(SIGSEGV, &sa, NULL);
}

static struct termios term;
static tcflag_t lflag;

static void secure_terminal(void)
{
  /* Disable terminal echoing and signals. */
  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~(ECHO | ISIG);
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

static void restore_terminal(void)
{
  /* Restore the terminal. */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

static int auth_tries;

static void auth_loop(const char *username)
{
  GError *err = NULL;
  struct timespec *prompt_timeout;
  struct timespec *wait_timeout;
  char *vlock_message;
  const char *auth_names[] = { username, "root", NULL };

  /* If NO_ROOT_PASS is defined or the username is "root" ... */
#ifndef NO_ROOT_PASS
  if (strcmp(username, "root") == 0)
#endif
    /* ... do not fall back to "root". */
    auth_names[1] = NULL;

  /* Get the vlock message from the environment. */
  vlock_message = getenv("VLOCK_MESSAGE");

  if (vlock_message == NULL) {
    if (console_switch_locked)
      vlock_message = getenv("VLOCK_ALL_MESSAGE");
    else
      vlock_message = getenv("VLOCK_CURRENT_MESSAGE");
  }

  /* Get the timeouts from the environment. */
  prompt_timeout = parse_seconds(getenv("VLOCK_PROMPT_TIMEOUT"));
#ifdef USE_PLUGINS
  wait_timeout = parse_seconds(getenv("VLOCK_TIMEOUT"));
#else
  wait_timeout = NULL;
#endif

  for (;;) {
    char c;

    /* Print vlock message if there is one. */
    if (vlock_message && *vlock_message) {
      fputs(vlock_message, stderr);
      fputc('\n', stderr);
    }

    /* Wait for enter or escape to be pressed. */
    c = wait_for_character("\n\033", wait_timeout);

    /* Escape was pressed or the timeout occurred. */
    if (c == '\033' || c == 0) {
#ifdef USE_PLUGINS
      plugin_hook("vlock_save");
      /* Wait for any key to be pressed. */
      c = wait_for_character(NULL, NULL);
      plugin_hook("vlock_save_abort");

      /* Do not require enter to be pressed twice. */
      if (c != '\n')
        continue;
#else
      continue;
#endif
    }

    for (size_t i = 0; auth_names[i] != NULL; i++) {
      if (auth(auth_names[i], prompt_timeout, &err))
        goto auth_success;

      g_assert(err != NULL);

      if (g_error_matches(err,
            VLOCK_PROMPT_ERROR,
            VLOCK_PROMPT_ERROR_TIMEOUT)) {
        fprintf(stderr, "Timeout!\n");
      } else {
        fprintf(stderr, "vlock: %s\n", err->message);

        if (g_error_matches(err,
              VLOCK_AUTH_ERROR,
              VLOCK_AUTH_ERROR_FAILED)) {
          fputc('\n', stderr);
          fprintf(stderr, "******************************************************************\n");
          fprintf(stderr, "*** You may not be able to able to unlock your terminal now.   ***\n");
          fprintf(stderr, "***                                                            ***\n");
          fprintf(stderr, "*** Log into another terminal and kill the vlock-main process. ***\n");
          fprintf(stderr, "******************************************************************\n");
          fputc('\n', stderr);
          sleep(3);
        }
      }

      g_clear_error(&err);
      sleep(1);
    }

    auth_tries++;
  }

auth_success:
  /* Free timeouts memory. */
  free(wait_timeout);
  free(prompt_timeout);
}

void display_auth_tries(void)
{
  if (auth_tries > 0)
    fprintf(stderr, "%d failed authentication %s.\n", auth_tries, auth_tries > 1 ? "tries" : "try");
}

#ifdef USE_PLUGINS
static void call_end_hook(void)
{
  (void) plugin_hook("vlock_end");
}
#endif

/* Lock the current terminal until proper authentication is received. */
int main(int argc, char *const argv[])
{
  const char *username = NULL;

  /* Initialize GLib. */
  g_set_prgname("vlock");

  /* Initialize logging. */
  vlock_initialize_logging();

  install_signal_handlers();

  /* Get the user name from the environment if started as root. */
  if (getuid() == 0)
    username = g_getenv("USER");

  if (username == NULL)
    username = g_get_user_name();

  ensure_atexit(display_auth_tries);

#ifdef USE_PLUGINS
  for (int i = 1; i < argc; i++)
    if (!load_plugin(argv[i]))
      g_critical("loading plugin '%s' failed: %s", argv[i], STRERROR);

  ensure_atexit(unload_plugins);

  if (!resolve_dependencies()) {
    if (errno == 0)
      exit(EXIT_FAILURE);
    else
      g_critical("error resolving plugin dependencies: %s", STRERROR);
  }

  plugin_hook("vlock_start");
  ensure_atexit(call_end_hook);
#else /* !USE_PLUGINS */
  /* Emulate pseudo plugin "all". */
  if (argc == 2 && (strcmp(argv[1], "all") == 0)) {
    if (!lock_console_switch()) {
      if (errno)
        perror("vlock: could not disable console switching");

      exit(EXIT_FAILURE);
    }

    ensure_atexit((void (*)(void))unlock_console_switch);
  } else if (argc > 1) {
    g_critical("plugin support disabled");
  }
#endif

  if (!isatty(STDIN_FILENO))
    g_critical("stdin is not a terminal");

  /* Delay securing the terminal until here because one of the plugins might
   * have changed the active terminal. */
  secure_terminal();
  ensure_atexit(restore_terminal);

  auth_loop(username);

  exit(0);
}

/* script.c -- script routines for vlock, the VT locking program for linux
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

/* Scripts are executables that are run as unprivileged child processes of
 * vlock.  They communicate with vlock through stdin and stdout.
 *
 * When dependencies are retrieved they are launched once for each dependency
 * and should print the names of the plugins they depend on on stdout one per
 * line.  The dependency requested is given as a single command line argument.
 *
 * In hook mode the script is called once with "hooks" as a single command line
 * argument.  It should not exit until its stdin closes.  The hook that should
 * be executed is written to its stdin on a single line.
 *
 * Currently there is no way for a script to communicate errors or even success
 * to vlock.  If it exits it will linger as a zombie until the plugin is
 * destroyed.
 */

#if !defined(__FreeBSD__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include "list.h"
#include "process.h"
#include "util.h"

#include "plugin.h"

static bool init_script(struct plugin *p);
static void destroy_script(struct plugin *p);
static bool call_script_hook(struct plugin *p, const char *hook_name);

struct plugin_type *script = &(struct plugin_type){
  .init = init_script,
  .destroy = destroy_script,
  .call_hook = call_script_hook,
};

struct script_context 
{
  /* The pipe file descriptor that is connected to the script's stdin. */
  int fd;
  /* The PID of the script. */
  pid_t pid;
};

/* Get the dependency from the script. */ 
static bool get_dependency(const char *path, const char *dependency_name,
    struct list *dependency_list);
/* Launch the script creating a new script_context.  No error detection aborts
 * on fatal errors. */
static struct script_context *launch_script(const char *path);

bool init_script(struct plugin *p)
{
  char *path;

  if (asprintf(&path, "%s/%s", VLOCK_SCRIPT_DIR, p->name) < 0) {
    errno = ENOMEM;
    return false;
  }

  /* Test for access.  This must be done manually because vlock most likely
   * runs as a setuid executable and would otherwise override restrictions.
   *
   * Also there is currently no error detection in case exec() fails later.
   */
  if (access(path, X_OK) < 0) {
    free(path);
    return false;
  }

  /* Get the dependency information. */
  for (size_t i = 0; i < nr_dependencies; i++)
    if (!get_dependency(path, dependency_names[i], p->dependencies[i]))
      return false;

  /* Launch the script. */
  p->context = launch_script(path);

  free(path);

  return true;
}

static void destroy_script(struct plugin *p)
{
  struct script_context *context = p->context;

  if (context != NULL) {
    /* Close the pipe. */
    (void) close(context->fd);

    /* Kill the child process. */
    if (!wait_for_death(context->pid, 0, 500000L))
      ensure_death(context->pid);

    free(context);
  }
}

/* Invoke the hook by writing it on a single line to the scripts stdin. */
static bool call_script_hook(struct plugin *s, const char *hook_name)
{
  struct script_context *context = s->context;
  char *data;
  ssize_t data_length;
  ssize_t length;
  struct sigaction act, oldact;

  /* Format the line. */
  data_length = asprintf(&data, "%s\n", hook_name);

  if (data_length < 0)
    fatal_error("memory allocation failed");

  /* When writing to a pipe when the read end is closed the kernel invariably
   * sends SIGPIPE.   Ignore it. */
  (void) sigemptyset(&(act.sa_mask));
  act.sa_flags = SA_RESTART;
  act.sa_handler = SIG_IGN;
  (void) sigaction(SIGPIPE, &act, &oldact);

  /* Send hook name and a newline through the pipe. */
  length = write(context->fd, data, data_length);

  /* Restore the previous SIGPIPE handler. */
  (void) sigaction(SIGPIPE, &oldact, NULL);

  /* Scripts fail silently. */
  return (length == data_length);
}

static struct script_context *launch_script(const char *path)
{
  struct script_context *script = ensure_malloc(sizeof *script);
  const char *argv[] = { path, "hooks", NULL };
  struct child_process child = {
    .path = path,
    .argv = argv,
    .stdin_fd = REDIRECT_PIPE,
    .stdout_fd = REDIRECT_PIPE,
    .stderr_fd = REDIRECT_PIPE,
    .function = NULL,
  };

  create_child(&child);
  script->fd = child.stdin_fd;
  script->pid = child.pid;

  return script;
}

static char *read_dependency(const char *path, const char *dependency_name);
static bool parse_dependency(char *data, struct list *dependency_list);

/* Get the dependency from the script. */
static bool get_dependency(const char *path, const char *dependency_name,
    struct list *dependency_list)
{
  /* Read the dependency data. */
  char *data;
  errno = 0;
  data = read_dependency(path, dependency_name);

  if (data == NULL)  {
    return errno != 0;
  } else {
    /* Parse the dependency data. */
    bool result = parse_dependency(data, dependency_list);
    int errsv = errno;
    free(data);
    errno = errsv;
    return result;
  }
}

/* Read the dependency data by starting the script with the name of the
 * dependency as a single command line argument.  The script should then print
 * the dependencies to its stdout one on per line. */
static char *read_dependency(const char *path, const char *dependency_name)
{
  char *error = NULL;
  int pipe_fds[2];
  pid_t pid;
  struct timeval timeout = (struct timeval){1, 0};
  char *data = ensure_calloc(1, sizeof *data);
  size_t data_length = 0;

  /* Open a pipe for the script. */
  if (pipe(pipe_fds) < 0)
    fatal_error("pipe() failed");

  pid = fork();

  if (pid == 0) {
    /* Child. */
    int nullfd = open("/dev/null", O_RDWR);

    if (nullfd < 0)
      _exit(1);

    /* Redirect stdio. */
    (void) dup2(nullfd, STDIN_FILENO);
    (void) dup2(pipe_fds[1], STDOUT_FILENO);
    (void) dup2(nullfd, STDERR_FILENO);

    /* Close all other file descriptors. */
    close_all_fds();

    /* Drop privileges. */
    setgid(getgid());
    setuid(getuid());

    (void) execl(path, path, dependency_name, NULL);

    _exit(1);
  }
  
  /* Close write end of the pipe. */
  (void) close(pipe_fds[1]);

  if (pid < 0) {
    (void) close(pipe_fds[0]);
    free(data);
    fatal_error("fork() failed");
  }

  /* Read the dependency from the child.  Reading fails if either the timeout
   * elapses or more that LINE_MAX bytes are read. */
  for (;;) {
    struct timeval t = timeout;
    struct timeval t1;
    struct timeval t2;
    char buffer[LINE_MAX];
    ssize_t length;

    fd_set read_fds;

    FD_ZERO(&read_fds);
    FD_SET(pipe_fds[0], &read_fds);

    /* t1 is before select. */
    (void) gettimeofday(&t1, NULL);

    if (select(pipe_fds[0]+1, &read_fds, NULL, NULL, &t) != 1) {
timeout:
      asprintf(&error, "timeout while reading dependency '%s' from '%s", dependency_name, path);
      goto read_error;
    }

    /* t2 is after select. */
    (void) gettimeofday(&t2, NULL);

    /* Get the time that during select. */
    timersub(&t2, &t1, &t2);

    /* This is very unlikely. */
    if (timercmp(&t2, &timeout, >))
      goto timeout;

    /* Reduce the timeout. */
    timersub(&timeout, &t2, &timeout);

    /* Read dependency data from the script. */
    length = read(pipe_fds[0], buffer, sizeof buffer - 1);

    /* Did the script close its stdin or exit? */
    if (length <= 0)
      break;

    if (data_length+length+1 > LINE_MAX) {
      asprintf(&error, "too much data while reading dependency '%s' from '%s'", dependency_name, path);
      goto read_error;
    }

    /* Append the buffer to the data string. */
    data = ensure_realloc(data, data_length+length);
    strncpy(data+data_length, buffer, length);
    data_length += length;
  }

  /* Terminate the data string. */
  data[data_length] = '\0';

  /* Close the read end of the pipe. */
  (void) close(pipe_fds[0]);
  /* Kill the script. */
  if (!wait_for_death(pid, 0, 500000L))
    ensure_death(pid);

  return data;

read_error:
  free(data);
  (void) close(pipe_fds[0]);
  if (!wait_for_death(pid, 0, 500000L))
    ensure_death(pid);
  fprintf(stderr, "%s\n", error);
  free(error);
  abort();
}

static bool parse_dependency(char *data, struct list *dependency_list)
{
  for (char *s = data, *saveptr;; s = NULL) {
    char *token = strtok_r(s, "\n", &saveptr);

    if (token == NULL)
      break;

    list_append(dependency_list, ensure_not_null(strdup(token), "could not copy string"));
  }

  return true;
}

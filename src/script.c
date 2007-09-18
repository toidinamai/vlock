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

#include "list.h"
#include "process.h"
#include "util.h"

#include "plugin.h"

#define VLOCK_SCRIPT_DIR PREFIX "/lib/vlock/scripts"

struct script_context 
{
  int fd;
  pid_t pid;
};

static void get_dependency(const char *path, const char *dependency_name,
    struct list *dependency_list);
static char *read_dependency(const char *path, const char *dependency_name);
static void parse_dependency(char *data, struct list *dependency_list);
static struct script_context *launch_script(const char *path);
static void close_script(struct plugin *s);
static bool call_script_hook(struct plugin *s, const char *hook_name);

struct plugin *open_script(const char *name, char **error)
{
  char *path;
  struct plugin *s = __allocate_plugin(name);

  if (asprintf(&path, "%s/%s", VLOCK_SCRIPT_DIR, name) < 0) {
    *error = strdup("filename too long");
    goto path_error;
  }

  if (access(path, X_OK) < 0) {
    (void) asprintf(error, "%s: %s", path, strerror(errno));
    goto access_error;
  }

  for (size_t i = 0; i < nr_dependencies; i++)
    get_dependency(path, dependency_names[i], s->dependencies[i]);

  s->context = launch_script(path);
  s->close = close_script;
  s->call_hook = call_script_hook;

  free(path);

  return s;

access_error:
  free(path);

path_error:
  __destroy_plugin(s);
  return NULL;
}

static bool call_script_hook(struct plugin *s, const char *hook_name)
{
  struct script_context *context = s->context;
  char *data;
  ssize_t data_length;
  ssize_t length;
  struct sigaction act, oldact;

  data_length = asprintf(&data, "%s\n", hook_name);

  if (data_length < 0)
    fatal_error("memory allocation failed");

  // ignore SIGPIPE
  (void) sigemptyset(&(act.sa_mask));
  act.sa_flags = SA_RESTART;
  act.sa_handler = SIG_IGN;
  (void) sigaction(SIGPIPE, &act, &oldact);

  // send hook name and line feed through the pipe
  length = write(context->fd, data, data_length);

  // restore SIGPIPE handler
  (void) sigaction(SIGPIPE, &oldact, NULL);

  // scripts fail silently
  return (length == data_length);
}

static void get_dependency(const char *path, const char *dependency_name,
    struct list *dependency_list)
{
  char *data = read_dependency(path, dependency_name);

  if (data != NULL)  {
    parse_dependency(data, dependency_list);
    free(data);
  }
}

static char *read_dependency(const char *path, const char *dependency_name)
{
  char *error = NULL;
  int pipe_fds[2];
  pid_t pid;
  struct timeval timeout;
  char *data = ensure_calloc(1, sizeof *data);
  size_t data_length = 0;

  // open a pipe for the script
  if (pipe(pipe_fds) < 0)
    fatal_error("pipe() failed");

  pid = fork();

  if (pid == 0) {
    // child
    int nullfd = open("/dev/null", O_RDWR);

    if (nullfd < 0)
      _exit(1);

    // redirect stdio
    (void) dup2(nullfd, STDIN_FILENO);
    (void) dup2(pipe_fds[1], STDOUT_FILENO);
    (void) dup2(nullfd, STDERR_FILENO);

    // close all other file descriptors
    close_all_fds();

    // drop privileges
    setgid(getgid());
    setuid(getuid());

    (void) execl(path, path, dependency_name, NULL);

    _exit(1);
  }

  (void) close(pipe_fds[1]);

  if (pid < 0) {
    (void) close(pipe_fds[0]);
    fatal_error("fork() failed");
  }

  for (;;) {
    char buffer[LINE_MAX];
    ssize_t len;

    fd_set read_fds;

    FD_ZERO(&read_fds);
    FD_SET(pipe_fds[0], &read_fds);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (select(pipe_fds[0]+1, &read_fds, NULL, NULL, &timeout) != 1) {
      asprintf(&error, "timeout while reading dependency '%s' from '%s", dependency_name, path);
      goto read_error;
    }

    len = read(pipe_fds[0], buffer, sizeof buffer - 1);

    if (len <= 0)
      break;

    if (data_length+len > LINE_MAX) {
      asprintf(&error, "too much data while reading dependency '%s' from '%s'", dependency_name, path);
      goto read_error;
    }

    data = ensure_realloc(data, data_length+len);

    strncpy(data+data_length, buffer, len);
    data_length += len;
  }

  data[data_length] = '\0';

  (void) close(pipe_fds[0]);
  ensure_death(pid);

  return data;

read_error:
  free(data);
  (void) close(pipe_fds[0]);
  ensure_death(pid);
  fputs(error, stderr);
  free(error);
  abort();
}

static void parse_dependency(char *data, struct list *dependency_list)
{
  for (char *s = data, *saveptr;; s = NULL) {
    char *token = strtok_r(s, "\n", &saveptr);

    if (token == NULL)
      break;

    list_append(dependency_list, ensure_not_null(strdup(token), "could not copy string"));
  }
}

static struct script_context *launch_script(const char *path)
{
  struct script_context *script = ensure_malloc(sizeof *script);
  int pipe_fds[2];

  if (pipe(pipe_fds) < 0)
    fatal_error("pipe() failed");

  script->pid = fork();
  script->fd = pipe_fds[1];

  if (script->pid == 0) {
    // child
    int nullfd = open("/dev/null", O_RDWR);

    if (nullfd < 0)
      _exit(1);

    // redirect stdio
    (void) dup2(pipe_fds[0], STDIN_FILENO);
    (void) dup2(nullfd, STDOUT_FILENO);
    (void) dup2(nullfd, STDERR_FILENO);

    // close all other file descriptors
    close_all_fds();

    // drop privileges
    setgid(getgid());
    setuid(getuid());

    (void) execl(path, path, "hooks", NULL);

    _exit(1);
  }

  (void) close(pipe_fds[0]);

  if (script->pid > 0) {
    return script;
  } else {
    free(script);
    fatal_error("fork() failed");
  }
}

static void close_script(struct plugin *s)
{
  struct script_context *context = s->context;

  (void) close(context->fd);
  if (!wait_for_death(context->pid, 0, 500000L))
    ensure_death(context->pid);
  free(context);
}

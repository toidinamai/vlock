#include <unistd.h>
#include <limits.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "script.h"

/* hard coded paths */
#define VLOCK_SCRIPT_DIR PREFIX "/lib/vlock/scripts"

static void get_dependency(const char *path, const char *name, list<string>& dependency);
static pid_t launch_script(const char *path, int pipe_fd);
static bool wait_for_death(pid_t pid, long sec, long usec);
static void ensure_death(pid_t pid);

Script::Script(string name) : Plugin(name)
{
  char path[FILENAME_MAX];
  int pipe_fds[2];

  /* format the plugin path */
  if (snprintf(path, sizeof path, "%s/%s", VLOCK_SCRIPT_DIR, name.c_str()) > (ssize_t)sizeof path)
    throw PluginException("plugin '" + name + "' filename too long");

  if (access(path, R_OK | X_OK) != 0)
    throw PluginException(string(path) + ": " + strerror(errno));

  /* load dependencies */
  for (vector<string>::iterator it = dependency_names.begin();
      it != dependency_names.end(); it++)
      get_dependency(path, (*it).c_str(), dependencies[*it]);

  if (pipe(pipe_fds) < 0)
    throw PluginException("pipe() failed");

  fd = pipe_fds[1];

  // set non-blocking mode
  (void) fcntl(fd, F_SETFL, O_NONBLOCK);

  pid = launch_script(path, pipe_fds[0]);
}

Script::~Script()
{
  (void) close(fd);
  if (!wait_for_death(pid, 0, 500000L))
    ensure_death(pid);
}

void Script::call_hook(string name)
{
  ssize_t length = name.length() + 1;
  ssize_t wlength;
  struct sigaction act;
  struct sigaction oldact;

  // ignore SIGPIPE
  (void) sigemptyset(&(act.sa_mask));
  act.sa_flags = SA_RESTART;
  act.sa_handler = SIG_IGN;
  (void) sigaction(SIGPIPE, &act, &oldact);

  wlength = write(fd, (name + "\n").data(), length);

  // restore SIGPIPE handler
  (void) sigaction(SIGPIPE, &oldact, NULL);

  if (wlength != length)
    throw PluginException("error calling hook '" + name + "' for script '" + this->name + "'");
}

bool close_all_fds(void)
{
  rlimit r;

  if (getrlimit(RLIMIT_NOFILE, &r) < 0)
    return false;

  // close all file descriptors except stdio
  for (unsigned int i = 0; i < r.rlim_cur; i++) {
    switch (i) {
      case STDIN_FILENO:
      case STDOUT_FILENO:
      case STDERR_FILENO:
        break;
      default:
        close(i);
    }
  }

  return true;
}

static void get_dependency(const char *path, const char *name, list<string>& dependency)
{
  int pipe_fds[2];
  pid_t pid;
  timeval timeout;
  string data;
  string errmsg;

  if (pipe(pipe_fds) < 0)
    throw PluginException("pipe() failed");

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
    if (!close_all_fds())
      _exit(1);

    // drop privileges
    setgid(getgid());
    setuid(getuid());

    (void) execl(path, path, name, NULL);

    _exit(1);
  }

  (void) close(pipe_fds[1]);

  if (pid < 0) {
    (void) close(pipe_fds[0]);
    throw PluginException("fork() failed");
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
      errmsg = errmsg + "timeout while reading dependency '" + name + "' from '" + path + "'";
      goto out;
    }

    len = read(pipe_fds[0], buffer, sizeof buffer - 1);

    if (len <= 0)
      break;

    buffer[len] = '\0';
    data += buffer;

    if (data.length() > LINE_MAX) {
      errmsg = errmsg + "too much data while reading dependency '" + name + "' from '" + path + "'";
      goto out;
    }
  }

  for (size_t pos1 = 0, pos2 = data.find('\n', pos1);
      pos1 < data.length();
      pos1 = pos2+1, pos2 = data.find('\n', pos1))
    if ((pos2 - pos1) > 1)
      dependency.push_back(data.substr(pos1, pos2 - pos1));

out:
  (void) close(pipe_fds[0]);
  ensure_death(pid);

  if (errmsg.length() != 0)
    throw PluginException(errmsg);
}

static pid_t launch_script(const char *path, int pipe_fd)
{
  pid_t pid = fork();

  if (pid == 0) {
    // child
    int nullfd = open("/dev/null", O_RDWR);

    if (nullfd < 0)
      _exit(1);

    // redirect stdio
    (void) dup2(pipe_fd, STDIN_FILENO);
    (void) dup2(nullfd, STDOUT_FILENO);
    (void) dup2(nullfd, STDERR_FILENO);

    // close all other file descriptors
    if (!close_all_fds())
      _exit(1);

    // drop privileges
    setgid(getgid());
    setuid(getuid());

    (void) execl(path, path, "hooks", NULL);

    _exit(1);
  }

  (void) close(pipe_fd);

  if (pid > 0)
    return pid;
  else
    throw PluginException("fork() failed");
}

static void handle_alarm(int __attribute__((unused)) signum)
{
  // ignore
}

static bool wait_for_death(pid_t pid, long sec, long usec)
{
  int status;

  if (sec > 0 || usec > 0) {
    struct sigaction act, oldact;
    struct itimerval timer, otimer;
    bool result;

    // ignore SIGALRM
    sigemptyset(&act.sa_mask);
    act.sa_handler = handle_alarm;
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, &oldact);

    // initialize timer
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 500000L;

    // set timer
    setitimer(ITIMER_REAL, &timer, &otimer);

    // wait until interupted by timer
    result = (waitpid(pid, &status, 0) == pid);

    // restore signal handler
    sigaction(SIGALRM, &oldact, NULL);

    // restore timer
    setitimer(ITIMER_REAL, &otimer, NULL);

    return result;
  } else {
    return (waitpid(pid, &status, WNOHANG) == pid);
  }
}

static void ensure_death(pid_t pid)
{
  int status;

  if (waitpid(pid, &status, WNOHANG) == pid)
    return;

  (void) kill(pid, SIGTERM);

  if (wait_for_death(pid, 0, 500000L))
    return;

  (void) kill(pid, SIGKILL);
  (void) kill(pid, SIGCONT);

  (void) waitpid(pid, &status, 0);
}

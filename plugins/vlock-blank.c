#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/tiocl.h>

#include "vlock_plugin.h"

const char *depends[] = { "vlock-all", NULL };

int vlock_save(void __attribute__((__unused__)) **ctx) {
  char arg[] = {TIOCL_BLANKSCREEN, 0};
  return ioctl(STDIN_FILENO, TIOCLINUX, arg);
}

int vlock_save_abort(void __attribute__((__unused__)) **ctx) {
  char arg[] = {TIOCL_UNBLANKSCREEN, 0};
  return ioctl(STDIN_FILENO, TIOCLINUX, arg);
}

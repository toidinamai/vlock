#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/tiocl.h>

void vlock_save(void __attribute__((__unused__)) **ctx) {
  char arg[] = {TIOCL_BLANKSCREEN, 0};
  (void) ioctl(STDIN_FILENO, TIOCLINUX, arg);
}

void vlock_save_abort(void __attribute__((__unused__)) *ctx) {
  char arg[] = {TIOCL_UNBLANKSCREEN, 0};
  (void) ioctl(STDIN_FILENO, TIOCLINUX, arg);
}

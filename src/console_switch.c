#if !defined(__FreeBSD__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/consio.h>
#else
#include <sys/vt.h>
#endif

#include "console_switch.h"

bool console_switch_locked = false;

/* This handler is called by a signal whenever a user tries to
 * switch away from this virtual console. */
static void release_vt(int __attribute__ ((__unused__)) signum)
{
  /* Deny console switch. */
  (void) ioctl(STDIN_FILENO, VT_RELDISP, 0);
}

/* This handler is called whenever a user switches to this
 * virtual console. */
static void acquire_vt(int __attribute__ ((__unused__)) signum)
{
  /* Acknowledge console switch. */
  (void) ioctl(STDIN_FILENO, VT_RELDISP, VT_ACKACQ);
}

static struct vt_mode vtm;

bool lock_console_switch(char **error)
{
  struct vt_mode lock_vtm;
  struct sigaction sa;

  /* Get the virtual console mode. */
  if (ioctl(STDIN_FILENO, VT_GETMODE, &vtm) < 0) {
    if (errno == ENOTTY || errno == EINVAL)
      *error = strdup("this terminal is not a virtual console\n");
    else
      (void) asprintf(error, "could not get virtual console mode: %s", strerror(errno));

    return false;
  }

  /* Use a copy of the current virtual console mode. */
  lock_vtm = vtm;

  (void) sigemptyset(&(sa.sa_mask));
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = release_vt;
  (void) sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = acquire_vt;
  (void) sigaction(SIGUSR2, &sa, NULL);

  /* set terminal switching to be process governed */
  lock_vtm.mode = VT_PROCESS;
  /* set terminal release signal, i.e. sent when switching away */
  lock_vtm.relsig = SIGUSR1;
  /* set terminal acquire signal, i.e. sent when switching here */
  lock_vtm.acqsig = SIGUSR2;
  /* set terminal free signal, not implemented on either FreeBSD or Linux */
  /* Linux ignores it but FreeBSD wants a valid signal number here */
  lock_vtm.frsig = SIGHUP;

  /* set virtual console mode to be process governed
   * thus disabling console switching */
  if (ioctl(STDIN_FILENO, VT_SETMODE, &lock_vtm) < 0) {
    (void) asprintf(error, "could not set virtual console mode: %s", strerror(errno));
    return false;
  }

  console_switch_locked = true;
  return true;
}

void unlock_console_switch(void)
{
  /* restore virtual console mode */
  if (ioctl(STDIN_FILENO, VT_SETMODE, vtm) < 0)
    perror("vlock-all: could not restore console mode");
}

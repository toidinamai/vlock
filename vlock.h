/* vlock.h -- main header file for vlock, the VT locking program for linux
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

#define O_PATTERN 1
#define O_VERSION 2
#define O_HELP    3


void release_vt(int signo);
void acquire_vt(int signo);
void set_signal_mask(int save);
void mask_signals(void);
void restore_signals(void);
void set_terminal(int print);
void restore_terminal(void);
void get_password(void);
void init_passwords(void);
void print_help(int exitcode);


/* Global variables: */

/* Option globals: */
  /* This determines whether the default behavior is to lock only the */
  /* current VT or all of them.  0 means current, 1 means all. */
  extern int o_lock_all;

/* Other globals: */
  /* Copy of the VT mode when the program was started */
  extern struct vt_mode ovtm;
  extern struct termios oterm;
  extern int vfd;
  extern int is_vt;

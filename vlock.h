/* vlock.h -- main header file for vlock, the VT locking program for linux
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software, which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

/* RCS log:
 * $Log: vlock.h,v $
 * Revision 1.6  1994/03/23  17:02:59  johnsonm
 * This time *really* removed the pattern-list support...
 *
 * Revision 1.5  1994/03/23  17:01:54  johnsonm
 * Removed appendages for pattern display.
 * Added support for non-VT ttys.
 *
 * Revision 1.4  1994/03/19  14:29:58  johnsonm
 * No longer need to deal with two-process model.
 *
 * Revision 1.3  1994/03/16  20:13:33  johnsonm
 * Added ignore_sigchld() for second process.  Not sure if that is
 * the right thing yet, since it doesn't seem to work quite.
 *
 * Revision 1.2  1994/03/15  18:27:33  johnsonm
 * Made consistent with all the changes in the other files...
 *
 * Revision 1.1  1994/03/13  16:28:16  johnsonm
 * Initial revision
 *
 */

static char rcsid_vlockh[] = "$Id: vlock.h,v 1.7 1994/07/03 12:09:48 johnsonm Exp $";


#define O_PATTERN 1
#define O_VERSION 2
#define O_HELP    3


void release_vt(int signo);
void acquire_vt(int signo);
void set_signal_mask(int save);
void mask_signals(void);
void restore_signals(void);
void set_terminal(void);
void restore_terminal(void);
void get_password(void);


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

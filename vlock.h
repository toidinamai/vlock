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
 * $Log$
 */

static char rcsid_vlockh[] = "$Id: vlock.h,v 1.1 1994/03/13 16:28:16 johnsonm Exp $";

/* Pattern lists are not yet really supported, but we'll put in the */
/* infrastructure for when they are. */
typedef struct __pattern {
  char *name;
} Pattern;

#define O_PATTERN 1
#define O_VERSION 2
#define O_HELP    3



void release_vt(int signo);
void acquire_vt(int signo);
void mask_signals(void);
void restore_signals(void);



/* Global variables: */

/* Option globals: */
  /* This determines whether the default behavior is to lock only the */
  /* current VT or all of them.  0 means current, 1 means all. */
  extern int o_lock_all;
  /* Pattern lists are not yet really supported, but we'll put in the */
  /* infrastructure for when they are. */
  extern Pattern *o_pattern_list;

/* Other globals: */
  /* Copy of the VT mode when the program was started */
  extern struct vt_mode ovtm;


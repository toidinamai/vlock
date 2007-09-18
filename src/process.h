/* process.h -- header for child process routines for vlock,
 *              the VT locking program for linux
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

#include <stdbool.h>
#include <sys/types.h>

/* Wait for the given amount of time for the death of the given child process.
 * If the child process dies in the given amount of time or already was dead
 * true is returned and false otherwise. */
bool wait_for_death(pid_t pid, long sec, long usec);

/* Try hard to kill the given child process. */
void ensure_death(pid_t pid);

/* Close all possibly open file descriptors except STDIN_FILENO, STDOUT_FILENO
 * and STDERR_FILENO. */
void close_all_fds(void);

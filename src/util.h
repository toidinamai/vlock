/* util.h -- header for utility routines for vlock,
 *           the VT locking program for linux
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

struct timespec;

/* Parse the given string (interpreted as seconds) into a
 * timespec.  On error NULL is returned.  The caller is responsible
 * to free the result.   The string may be NULL, in which case NULL
 * is returned, too. */
struct timespec *parse_seconds(const char *s);

void __attribute__((noreturn)) fatal_error(const char *format, ...);

#define ensure_atexit(func) \
  do { \
    if (atexit(func) != 0) \
      fatal_error("vlock-main: Cannot register function '%s' with atexit().\n",  #func); \
  } while (0)

/* util.c -- utility routines for vlock, the VT locking program for linux
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

#if !defined(__FreeBSD__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "util.h"

/* Parse the given string (interpreted as seconds) into a
 * timespec.  On error NULL is returned.  The caller is responsible
 * to free the result.   The string may be NULL, in which case NULL
 * is returned, too. */
struct timespec *parse_seconds(const char *s)
{
  if (s == NULL) {
    return NULL;
  } else {
    char *n;
    struct timespec *t = calloc(sizeof *t, 1);

    if (t == NULL)
      return NULL;

    t->tv_sec = strtol(s, &n, 10);

    if (*n != '\0' || t->tv_sec <= 0) {
      free(t);
      t = NULL;
    }

    return t;
  }
}

void fatal_error(const char *format, ...)
{
  char *error;
  va_list ap;
  va_start(ap, format);
  if (vasprintf(&error, format, ap) < 0)
    error = "error while formatting error message";
  va_end(ap);
  fatal_error_free(error);
}

void fatal_error_free(char *error)
{
  fputs(error, stderr);
  fputc('\n', stderr);
  free(error);
  abort();
}

void *ensure_malloc(size_t size)
{
  void *r = malloc(size);

  if (r == NULL)
    fatal_error("failed to allocate %d bytes", size);

  return r;
}

void *ensure_calloc(size_t number, size_t size)
{
  void *r = calloc(number, size);

  if (r == NULL)
    fatal_error("failed to allocate %d bytes", size);

  return r;
}

void *ensure_realloc(void *ptr, size_t size)
{
  void *r = realloc(ptr, size);

  if (size != 0 && r == NULL)
    fatal_error("failed to reallocate %d bytes at %p", size, ptr);

  return r;
}

void *ensure_not_null(void *data, const char *errmsg)
{
  if (data == NULL)
    fatal_error(errmsg);

  return data;
}

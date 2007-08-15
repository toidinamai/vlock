/* prompt.c -- prompt routines for vlock,
 *             the VT locking program for linux
 *
 * This program is copyright (C) 2007 Frank Benkstein, and is free
 * software which is freely distributable under the terms of the
 * GNU General Public License version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 *
 * The prompt functions (prompt and prompt_echo_off) were
 * inspired by/copied from openpam's openpam_ttyconv.c:
 *
 * Copyright (c) 2002-2003 Networks Associates Technology, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "vlock.h"

#define PROMPT_BUFFER_SIZE 512

char *prompt(const char *msg, const struct timespec *timeout) {
  char buffer[PROMPT_BUFFER_SIZE];
  char *result;
  int len;
  struct termios term;
  struct timeval *timeout_val = NULL;
  tcflag_t lflag;
  fd_set readfds;

  if (timeout != NULL
      && (timeout_val = malloc(sizeof *timeout_val)) != NULL) {
    timeout_val->tv_sec = timeout->tv_sec;
    timeout_val->tv_usec = timeout->tv_nsec / 1000;
  }

  if (msg != NULL) {
    /* Write out the prompt. */
    (void) fputs(msg, stderr); fflush(stderr);
  }

  /* Get the current terminal attributes. */
  (void) tcgetattr(STDIN_FILENO, &term);
  /* Save the lflag value. */
  lflag = term.c_lflag;
  /* Enable canonical mode.  We're only interested in line buffering. */
  term.c_lflag |= ICANON;
  /* Disable terminal signals. */
  term.c_lflag &= ~ISIG;
  /* Set the terminal attributes. */
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
  /* Discard all unread input characters. */
  (void) tcflush(STDIN_FILENO, TCIFLUSH);

  /* Initialize file descriptor set. */
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  errno = 0;

  /* Wait until a string was entered. */
  if (select(STDIN_FILENO+1, &readfds, NULL, NULL, timeout_val) != 1) {
    if (errno)
      perror("vlock-auth: select() on stdin failed");
    else
      fprintf(stderr, "timeout!\n");

    result = NULL;
    goto out;
  }

  /* Read the string from stdin.  At most buffer length - 1 bytes, to
   * leave room for the terminating zero byte. */
  if ((len = read(STDIN_FILENO, buffer, sizeof buffer - 1)) < 0) {
    result = NULL;
    goto out;
  }

  /* Terminate the string. */
  buffer[len] = '\0';

  /* Strip trailing newline characters. */
  for (len = strlen(buffer); len > 0; --len) {
    if (buffer[len-1] != '\r' && buffer[len-1] != '\n')
      break;
  }

  /* Terminate the string, again. */
  buffer[len] = '\0';

  /* Copy the string. */
  result = strdup(buffer);

  /* Clear our buffer. */
  memset(buffer, 0, sizeof buffer);

out:
  free(timeout_val);

  /* Restore original terminal attributes. */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

  return result;
}

char *prompt_echo_off(const char *msg, const struct timespec *timeout) {
  struct termios term;
  tcflag_t lflag;
  char *result;

  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~ECHO;
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

  result = prompt(msg, timeout);

  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

  if (result != NULL)
    fputc('\n', stderr);

  return result;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#include "vlock.h"

#define PROMPT_BUFFER_SIZE 512

char *prompt(const char *msg) {
  char buffer[PROMPT_BUFFER_SIZE];
  char *result;
  int len;
  struct termios term;
  tcflag_t lflag;
  fd_set readfds;

  /* Write out the prompt. */
  (void) fputs(msg, stderr); fflush(stderr);

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

  /* Wait until a string was entered. */
  if (select(STDIN_FILENO+1, &readfds, NULL, NULL, NULL) != 1) {
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
  /* Restore original terminal attributes. */
  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

  return result;
}

char *prompt_echo_off(const char *msg) {
  struct termios term;
  tcflag_t lflag;
  char *result;

  (void) tcgetattr(STDIN_FILENO, &term);
  lflag = term.c_lflag;
  term.c_lflag &= ~ECHO;
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

  result = prompt(msg);

  term.c_lflag = lflag;
  (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);

  if (result != NULL)
    fputc('\n', stderr);

  return result;
}

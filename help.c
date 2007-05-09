/* help.c -- print help and usage message.
 *
 * This program is copyright (C) 1994 Michael K. Johnson, and is free
 * software, which is freely distributable under the terms of the
 * GNU public license version 2, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vlock.h"


void print_help(int exitcode) {
  fprintf(stderr,
	  "vlock: locks virtual consoles, saving your current session.\n"
	  "Usage: vlock [options]\n"
	  "       Where [options] are any of:\n"
	  "-c or --current: lock only this virtual console, allowing user to\n"
	  "       switch to other virtual consoles.\n"
	  "-a or --all: lock all virtual consoles by preventing other users\n"
	  "       from switching virtual consoles.\n"
	  "-s or --disable-sysrq: disable sysrq while consoles are locked to\n"
	  "       prevent killing vlock with SAK, requires --all.\n"
	  "-v or --version: Print the version number of vlock and exit.\n"
	  "-h or --help: Print this help message and exit.\n"
	  );
	  exit(exitcode);
}

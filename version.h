/* version.h -- version header file for vlock
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
 * $Log: version.h,v $
 * Revision 1.6  1996/05/17 02:37:40  johnsonm
 * Marek's massive changes include proper support for shadow passwords.
 *
 * Revision 1.5  1994/07/03 13:23:42  johnsonm
 * Version 0.8 fixes several security bugs, including -a not working on
 *   second and subsequent tries at the password, and SIGINT possibly
 *   killing it, and Ctrl-Z giving it problems on some platforms.
 *
 * Revision 1.4  1994/03/21  17:41:59  johnsonm
 * Bumped to version 0.6 for root bug fix.
 *
 * Revision 1.3  1994/03/19  14:23:08  johnsonm
 * Bumped version to 0.5, since vlock actually works now...
 *
 * Revision 1.2  1994/03/13  17:31:16  johnsonm
 * Made VERSION say more than a silly number, added a newline.
 *
 * Revision 1.1  1994/03/13  16:28:16  johnsonm
 * Initial revision
 *
 */

static char rcsid_versionh[] = "$Id: version.h,v 1.7 1996/06/21 23:27:09 johnsonm Exp $";

#define VERSION "vlock version 1.0\n"

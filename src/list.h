/* list.h -- doubly linked list header file for vlock,
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

struct List {
  void *data;
  struct List *next;
  struct List *previous;
};

struct List *list_first(struct List *list);
struct List *list_next(struct List *list);
struct List *list_append(struct List *list, void *data);
struct List *list_remove(struct List *list, void *data);
struct List *list_find(struct List *list, void *data);
struct List *list_last(struct List *list);
struct List *list_previous(struct List *list);
struct List *list_copy(struct List *list);
void list_free(struct List *list);

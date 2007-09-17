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

struct list_item
{
  void *data;
  struct list_item *next;
  struct list_item *previous;
};

struct list
{
  struct list_item *first;
  struct list_item *last;
};

typedef void (*free_item_function)(void *);

struct list *list_new(void);
void list_free(struct list *l, free_item_function free_item);

size_t list_length(struct list *l);
void list_append(struct list *l, void *data);

struct list_item *list_delete_item(struct list *l, struct list_item *item);

#define list_for_each(list, item) \
  for (struct list_item *item = (list)->first; item != NULL; item = item->next)

#define list_for_each_manual(list, item) \
  for (struct list_item *item = (list)->first; item != NULL; )

#if 0
/* Inspired by the doubly linked list code from glib:
 *
 * GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/* An item of the list.
 *
 * Any item also represents the full list.
 *
 * An empty list is represented by the NULL pointer.
 */
struct List {
  void *data;
  struct List *next;
  struct List *previous;
};

/* Get the first item of the list.  Returns the first item of the list or NULL
 * if the list is empty. */
struct List *list_first(struct List *list);

/* Get the last item of the list.  Returns the last item of the list or NULL if
 * the list is empty. */
struct List *list_last(struct List *list);

/* Get the next item.  Returns the item after the given item or NULL if the
 * list is empty. */
struct List *list_next(struct List *list);

/* Get the previous item.  Returns the item before the given item or NULL if
 * the list is empty. */
struct List *list_previous(struct List *list);

/* Allocate a new list item with the given data pointer and append it to the
 * end of the list.  Returns the new start of the list.  Calls abort() on
 * memory errors. */
struct List *list_append(struct List *list, void *data);

/* Removes the first item with the given data pointer and deletes it from the
 * list.  Returns the new start of the list.  Does not free the data. */
struct List *list_remove(struct List *list, void *data);

/* Delete one item from the list.  Returns the new start of the list. */
struct List *list_delete_link(struct List *list, struct List *item);

/* Returns the first item with the given data pointer or NULL if none is found.
 */
struct List *list_find(struct List *list, void *data);

/* Make a copy of the list.  Returns a shallow copy of the entire list.  Calls
 * abort() on memory error. */
struct List *list_copy(struct List *list);

/* Free the entire list.  Does free the items'. */
void list_free(struct List *list);

#define list_for_each(list, item) \
  for (struct List *item = list_first(list); item != NULL; item = list_next(item))

#define list_reverse_for_each(list, item) \
    for (struct List *item = list_last(plugins); item != NULL; item = list_previous(item))
#endif

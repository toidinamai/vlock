/* list.c -- doubly linked list routines for vlock,
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

#include <stdlib.h>
#include <stdio.h>

#include "list.h"

/* Like list_append() but returns the item that was added instead of the start
 * of the list. */
static struct List *__list_append(struct List *list, void *data) {
  struct List *new_item = malloc(sizeof (struct List));
  struct List *last = list_last(list);

  if (new_item == NULL) {
    fprintf(stderr, "%s:%d failed to allocate new list item\n", __FILE__, __LINE__);
    abort();
  }

  new_item->data = data;
  new_item->next = NULL;
  new_item->previous = last;

  if (last != NULL)
    last->next = new_item;

  return new_item;
}

struct List *list_append(struct List *list, void *data) {
  struct List *new_item = __list_append(list, data);

  if (list != NULL)
    return list;
  else
    return new_item;
}

struct List *list_copy(struct List *list) {
  struct List *new_list = NULL;
  struct List *last = NULL;
  struct List *item = list_first(list);

  /* Make a copy of the first item.  It is now also the last item of the new
   * list. */
  if (item != NULL)
    last = new_list = list_append(new_list, item->data);

  /* Append all further items to the first. */
  for (item = list_next(item); item != NULL; item = list_next(item))
    last = __list_append(last, item->data);

  return list_first(new_list);
}

struct List *list_first(struct List *list) {
  if (list != NULL) {
    while (list->previous != NULL)
      list = list->previous;
  }

  return list;
}

struct List *list_last(struct List *list) {
  if (list != NULL) {
    while (list->next != NULL)
      list = list->next;
  }

  return list;
}

struct List *list_next(struct List *list) {
  if (list == NULL)
    return NULL;
  else
    return list->next;
}

struct List *list_previous(struct List *list) {
  if (list == NULL)
    return NULL;
  else
    return list->previous;
}

struct List *list_find(struct List *list, void *data) {
  list_for_each(list, item) {
    if (item->data == data)
      return item;
  }

  return NULL;
}

/* Delete one item from the list.  Returns the new start of the list. */
static struct List *list_delete_link(struct List *list, struct List *item) {
  if (item != NULL) {
    if (item->previous != NULL)
      item->previous->next = item->next;
    if (item->next != NULL)
      item->next->previous = item->previous;
    if (item == list) {
      if (item->next != NULL)
        list = item->next;
      else if (item->previous != NULL)
        list = item->previous;
      else
        list = NULL;
    }

    free(item);
  }

  return list;
}

struct List *list_remove(struct List *list, void *data) {
  struct List *item = list_find(list, data);
  return list_delete_link(list, item);
}

void list_free(struct List *list) {
  while (list != NULL)
    list = list_delete_link(list, list_first(list));
}

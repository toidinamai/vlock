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

#include "util.h"

#include "list.h"

struct list *list_new(void)
{
  struct list *l = ensure_malloc(sizeof *l);
  l->first = NULL;
  l->last = NULL;
  return l;
}

void list_free(struct list *l)
{
  list_for_each_manual(l, item) {
    struct list_item *tmp = item->next;
    free(item);
    item = tmp;
  }

  free(l);
}

void list_append(struct list *l, void *data)
{
  struct list_item *item = ensure_malloc(sizeof *item);

  item->data = data;
  item->previous = l->last;
  item->next = NULL;

  if (l->last != NULL)
    l->last->next = item;

  l->last = item;

  if (l->first == NULL)
    l->first = item;
}

struct list *list_copy(struct list *l)
{
  struct list *new_list = list_new();

  list_for_each(l, item)
    list_append(new_list, item->data);

  return new_list;
}

size_t list_length(struct list *l)
{
  size_t length = 0;

  list_for_each(l, item)
    length++;

  return length;
}

struct list_item *list_delete_item(struct list *l, struct list_item *item)
{
  struct list_item *next = item->next;

  if (item->previous != NULL)
    item->previous->next = item->next;

  if (item->next != NULL)
    item->next->previous = item->previous;

  if (l->first == item)
    l->first = item->next;

  if (l->last == item)
    l->last = item->previous;

  free(item);

  return next;
}

void list_delete(struct list *l, void *data)
{
  struct list_item *item = list_find(l, data);

  if (item != NULL)
    (void) list_delete_item(l, item);
}

struct list_item *list_find(struct list *l, void *data)
{
  list_for_each(l, item)
    if (item->data == data)
      return item;

  return NULL;
}

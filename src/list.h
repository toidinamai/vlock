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

#include <stdbool.h>
#include <stddef.h>

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

struct list *list_new(void);
struct list *list_copy(struct list *l);

void list_free(struct list *l);

size_t list_length(struct list *l);
void list_append(struct list *l, void *data);

struct list_item *list_delete_item(struct list *l, struct list_item *item);
void list_delete(struct list *l, void *data);

struct list_item *list_find(struct list *l, void *data);

#define list_for_each_from_increment(list, item, start, increment) \
  for (struct list_item *item = (start); item != NULL; (increment))

#define list_for_each_from(list, item, start) \
  for (struct list_item *item = (start); item != NULL;)

#define list_for_each(list, item) \
  list_for_each_from_increment((list), item, (list)->first, item = item->next)

#define list_delete_for_each(list, item) \
  list_for_each_from_increment((list), item, (list)->first, item = list_delete_item((list), item))

#define list_for_each_manual(list, item) \
  list_for_each_from((list), item, (list)->first)

#define list_for_each_reverse_from(list, item, start) \
  list_for_each_from_increment((list), item, (start), item = item->previous)

#define list_for_each_reverse(list, item) \
  list_for_each_reverse_from((list), item, (list)->last)

static inline bool list_is_empty(struct list *l)
{
  return l->first == NULL;
}

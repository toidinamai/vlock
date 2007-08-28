#include <stdlib.h>

#include "list.h"

static struct List *__list_append(struct List *list, void *data) {
  struct List *new_item = malloc(sizeof (struct List));
  struct List *last = list_last(list);

  if (new_item == NULL)
    abort();

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

  if (item != NULL)
    last = new_list = list_append(new_list, item->data);

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
  for (struct List *item = list_first(list); item != NULL; item = list_next(item)) {
    if (item->data == data)
      return item;
  }

  return NULL;
}

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

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

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <util/util.h>

typedef struct {
    void *data;
    size_t elem_size;
    size_t size;
    size_t capacity;
} List;

typedef enum  {
    LIST_EMPTY,
    OUT_OF_BOUNDS,
} ListError;

define_result(ListItemResult, void *, ListError);
define_result(ListOpResult, Unit, ListError);

List init_list(size_t elem_size);
void destroy_list(List *list);

void list_push(List *list, void *n);
ListItemResult list_at(List *list, size_t index);
ListOpResult list_place(List *list, size_t index, void *n);
ListItemResult list_pop(List *list);
void list_shuffle(List *list);
ListOpResult list_swap(List *list, size_t first, size_t second);

#endif
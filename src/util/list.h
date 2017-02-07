#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct {
    void *data;
    size_t elem_size;
    size_t size;
    size_t capacity;
} List;

List init_list(size_t elem_size);
void destroy_list(List *list);

void list_push(List *list, void *n);
void *list_at(List *list, size_t index);
void list_place(List *list, size_t index, void *n);
void *list_pop(List *list);
void list_shuffle(List *list);
void list_swap(List *list, size_t first, size_t second);

#endif
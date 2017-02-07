#ifndef HEAP_H
#define HEAP_H

#include <util/util.h>
#include <util/list.h>
typedef struct {
    List data;
    int (*comparator)(void* this, void* to);
} Heap;

Heap init_heap(int (*comparator)(void* this, void* to), size_t elem_size);
void insert_element(Heap *heap, void *n);
void *remove_top(Heap *heap);
void destroy_heap(Heap *heap);

#endif
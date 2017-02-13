#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>

#include <util/util.h>
#include <collections/list.h>
typedef struct {
    List data;
    int (*comparator)(void* this, void* to);
} Heap;

typedef enum  {
    HEAP_EMPTY
} HeapError;

define_result(HeapResult, void *, HeapError);

Heap init_heap(int (*comparator)(void* this, void* to), size_t elem_size);
void heap_push(Heap *heap, void *n);
HeapResult heap_pop(Heap *heap);
bool heap_empty(Heap *heap);
void destroy_heap(Heap *heap);

#endif
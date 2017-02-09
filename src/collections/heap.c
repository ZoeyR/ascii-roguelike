#include <collections/heap.h>
#include <collections/list.h>
#include <util/util.h>

static size_t _parent_index(size_t index);
static size_t _left_child_index(Heap *heap, size_t index);
static size_t _right_child_index(Heap *heap, size_t index);

Heap init_heap(int (*comparator)(void* this, void* to), size_t elem_size) {
    List list = init_list(elem_size);

    return (Heap){.data = list, .comparator = comparator};
}

void destroy_heap(Heap *heap) {
    destroy_list(&heap->data);
}

void heap_push(Heap *heap, void *n) {
    list_push(&heap->data, n);

    size_t our_index = heap->data.size - 1;
    size_t parent_index = _parent_index(our_index);

    while(heap->comparator(list_at(&heap->data, our_index), list_at(&heap->data, parent_index)) < 0) {
        list_swap(&heap->data, our_index, parent_index);
        our_index = parent_index;
        parent_index = _parent_index(our_index);
    }
}

void *heap_pop(Heap *heap) {
    list_swap(&heap->data, 0, heap->data.size - 1);
    void *ret = list_pop(&heap->data);

    size_t left_child_index = _left_child_index(heap, 0);
    size_t right_child_index = _right_child_index(heap, 0);
    size_t our_index = 0;

    while(heap->comparator(list_at(&heap->data, our_index), list_at(&heap->data, left_child_index)) > 0
          || heap->comparator(list_at(&heap->data, our_index), list_at(&heap->data, right_child_index)) > 0) {
        size_t smallest_child_index;
        if(heap->comparator(list_at(&heap->data, left_child_index), list_at(&heap->data, right_child_index)) > 0) {
            smallest_child_index = right_child_index;
        } else {
            smallest_child_index = left_child_index;
        }

        list_swap(&heap->data, our_index, smallest_child_index);
        our_index = smallest_child_index;
        left_child_index = _left_child_index(heap, our_index);
        right_child_index = _right_child_index(heap, our_index);
    }
    
    return ret;
}

bool heap_empty(Heap *heap) {
    return heap->data.size == 0;
}

static size_t _parent_index(size_t index) {
    if(index == 0) {
        return 0;
    }

    return (index - 1) / 2;
}
static size_t _left_child_index(Heap *heap, size_t index) {
    size_t left_child = (index * 2) + 1;
    return left_child < heap->data.size?left_child:index;
}

static size_t _right_child_index(Heap *heap, size_t index) {
    size_t right_child = (index * 2) + 2;
    return right_child < heap->data.size?right_child:index;
}
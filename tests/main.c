#include <stdio.h>
#include <stdint.h>

#include <collections/heap.h>
#include <util/distance.h>

int compare_int(void *this, void *to) {
    return *(int *)this - *(int *)to;
}

int test_heap_build() {
    int init_array[10] = {5, 77, 23, 9, 15, 48, 2, 2, 63, 33};
    int sort_array[10] = {2, 2, 5, 9, 15, 23, 33, 48, 63, 77};
    Heap heap = init_heap(compare_int, sizeof(int));

    for(int i = 0; i < 10; i++) {
        heap_push(&heap, (void *)&init_array[i]);
    }

    for(int i = 0; i < 10; i++) {
        int n = *(int *)heap_pop(&heap);

        if(n != sort_array[i]) {
            return 1;
        }
    }

    return 0;
}

int test_heap_starts_empty() {
    Heap heap = init_heap(compare_int, sizeof(int));
    if (!heap_empty(&heap)) {
        return 1;
    }

    return 0;
}

int test_heap_not_empty() {
    Heap heap = init_heap(compare_int, sizeof(int));
    int i = 1;
    heap_push(&heap, (void *)&i);

    if (heap_empty(&heap)) {
        return 1;
    }
    return 0;
}

int test_heap_becomes_empty() {
    Heap heap = init_heap(compare_int, sizeof(int));
    int i = 1;
    heap_push(&heap, (void *)&i);
    heap_pop(&heap);

    if (!heap_empty(&heap)) {
        return 1;
    }
    return 0;
}


static int _len(void *context, Coordinate *this, Coordinate *to) {
    (void)(context);
    (void)(this);
    (void)(to);
    return 1;
}

int test_dijkstra_no_segfault() {
    djikstra(NULL, 60, 60, _len);

    return 0;
}

#define test(test) ({ \
    printf("%s...", #test); \
    int result = test(); \
    if(result) { \
        printf("\033[31;40mFAIL\033[0m\n"); \
    } else { \
        printf("\033[32;40mOK\033[0m\n"); \
    } \
    result; \
})

int main() {
    int status = 0;
    status |= test(test_heap_build) << 0;
    status |= test(test_heap_starts_empty) << 1;
    status |= test(test_heap_not_empty) << 2;
    status |= test(test_heap_becomes_empty) << 3;
    status |= test(test_dijkstra_no_segfault) << 4;

    return status;
}


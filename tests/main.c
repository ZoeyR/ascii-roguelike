#include <stdio.h>
#include <stdint.h>

#include <util/heap.h>

int compare_int(void *this, void *to) {
    return *(int *)this - *(int *)to;
}

int test_heap_build() {
    int init_array[10] = {5, 77, 23, 9, 15, 48, 2, 2, 63, 33};
    int sort_array[10] = {2, 2, 5, 9, 15, 23, 33, 48, 63, 77};
    Heap heap = init_heap(compare_int, sizeof(int));

    for(int i = 0; i < 10; i++) {
        insert_element(&heap, (void *)&init_array[i]);
    }

    for(int i = 0; i < 10; i++) {
        int n = *(int *)remove_top(&heap);

        if(n != sort_array[i]) {
            return 1;
        }
    }

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

    return status;
}


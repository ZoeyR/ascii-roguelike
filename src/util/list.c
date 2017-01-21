#include <util/util.h>

IntList init_list(void) {
    size_t capacity = 10;
    size_t size = 0;
    int *data = malloc(sizeof(int) * capacity);

    return (IntList){.capacity = capacity, .size = size, .data = data};
}

void destroy_list(IntList *list) {
    list->size = 0;
    list->capacity = 0;
    free(list->data);
}

void list_push(IntList *list, int n) {
    list->data[list->size] = n;
    list->size++;

    if (list->size >= list->capacity) {
        // reallocate list
        int *new_data = realloc(list->data, sizeof(int) * (list->capacity * 2));
        if (!new_data) {
            // error! revert the push
            list->size--;
            return;
        }
        list->data = new_data;
        list->capacity = list->capacity * 2;
    }
}

int list_pop(IntList *list) {
    list->size--;
    return list->data[list->size];
}

void list_shuffle(IntList *list) {
    for (int i = 0; i < (int)list->size; i++) {
        int swap = i + better_rand(list->size - i - 1);
        int tmp = list->data[swap];
        list->data[swap] = list->data[i];
        list->data[i] = tmp;
    }
}

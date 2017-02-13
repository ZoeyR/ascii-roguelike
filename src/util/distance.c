#include <limits.h>
#include <stdio.h>

#include <collections/heap.h>
#include <dungeon/dungeon.h>
#include <util/distance.h>

typedef struct {
    int row;
    int col;
    int distance;
} HeapCoord;

static int _comp(void *this, void *to);

Distances djikstra(void *context, int start_row, int start_col, int (*length)(void *context, Coordinate* this, Coordinate* to)) {
    Heap heap;
    bool processed[DUNGEON_HEIGHT][DUNGEON_WIDTH];
    Distances d;

    heap = init_heap(_comp, sizeof(HeapCoord));
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            d.d[row][col] = INT_MAX;
            processed[row][col] = false;
        }
    }
    processed[start_row][start_col] = true;
    d.d[start_row][start_col] = 0;

    heap_push(&heap, &(HeapCoord){.row = start_row, .col = start_col, .distance = 0});

    while(!heap_empty(&heap)) {
        HeapCoord c = *(HeapCoord *)unwrap(heap_pop(&heap), 1);
        processed[c.row][c.col] = true;
        
        relative_array(1, c.row, c.col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );
        int adjacent[8][2] = {{top, left}   , {top, c.col}   , {top, right},
                              {c.row, left} ,                  {c.row, right},
                              {bottom, left}, {bottom, c.col}, {bottom, right}};
        
        for(int i = 0; i < 8; i++) {
            int row = adjacent[i][0];
            int col = adjacent[i][1];
            if (!processed[row][col]) {
                Coordinate c1 = {.row = c.row, .col = c.col};
                Coordinate c2 = {.row = row, .col = col};
                int len = length(context, &c1, &c2);
                if (len != INT_MAX) {
                    int alt = c.distance + len;
                
                    if (alt < d.d[row][col]) {
                        d.d[row][col] = alt;
                        heap_push(&heap, &(HeapCoord){.row = row, .col = col, .distance = alt});
                    }
                }
            }
        }
    }

    // Clean up data
    destroy_heap(&heap);

    return d;
}

static int _comp(void *this, void *to) {
    HeapCoord c_this = *(HeapCoord *)this;
    HeapCoord c_to = *(HeapCoord *)to;
    return c_this.distance - c_to.distance;
}
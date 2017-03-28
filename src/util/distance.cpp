#include <climits>

#include <collections/heap.h>
#include <dungeon/dungeon.h>
#include <util/distance.h>

typedef struct {
    int row;
    int col;
    int distance;
} HeapCoord;

Distances dijkstra(void *context, int start_row, int start_col, int (*length)(void *context, Coordinate* from, Coordinate* to)) {
    Heap<HeapCoord> heap([](auto from, auto to) {return from.distance - to.distance;});
    bool processed[DUNGEON_HEIGHT][DUNGEON_WIDTH];
    Distances d;

    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            d.d[row][col] = INT_MAX;
            processed[row][col] = false;
        }
    }
    processed[start_row][start_col] = true;
    d.d[start_row][start_col] = 0;

    heap.push((HeapCoord){.row = start_row, .col = start_col, .distance = 0});

    while(heap.is_empty()) {
        HeapCoord c = heap.pop();
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
                       heap.push((HeapCoord){.row = row, .col = col, .distance = alt});
                    }
                }
            }
        }
    }

    return d;
}
#ifndef LOOP_H
#define LOOP_H

#include <collections/heap.h>
#include <dungeon/entities.h>
#include <dungeon/dungeon.h>

typedef struct {
    DungeonBlock blocks[DUNGEON_HEIGHT][DUNGEON_WIDTH];
} View;

typedef struct {
    Heap event_queue;
    Dungeon dungeon;
    View view;
} GameState;


GameState init_state(Dungeon dungeon);
void destroy_state(GameState *state);
bool tick(GameState *state);
#endif
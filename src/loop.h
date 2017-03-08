#ifndef LOOP_H
#define LOOP_H

#include <collections/heap.h>
#include <dungeon/entities.h>
#include <dungeon/dungeon.h>

typedef struct {
    Heap event_queue;
    Dungeon dungeon;
} GameState;

GameState init_state(Dungeon dungeon);
void destroy_state(GameState *state);
bool tick(GameState *state);
#endif
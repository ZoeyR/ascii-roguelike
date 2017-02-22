#ifndef LOOP_H
#define LOOP_H

#include <collections/heap.h>
#include <dungeon/entities.h>
#include <dungeon/dungeon.h>

typedef struct {
    Heap event_queue;
    bool player_dead;
} GameState;

GameState init_state(Dungeon *dungeon);
bool tick(Dungeon *dungeon, GameState *state);
#endif
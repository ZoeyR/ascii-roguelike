#ifndef LOOP_H
#define LOOP_H

#include <collections/heap.h>
#include <dungeon/entities.h>
#include <dungeon/dungeon.h>

typedef struct {
    DungeonBlock blocks[DUNGEON_HEIGHT][DUNGEON_WIDTH];
} View;

typedef struct {
    int turn;
    EIdx entity_id;
    enum {
        MOVE
    } event_type;
} Event;

typedef struct {
    int row;
    int col;
} Coord;

class GameState {
    Heap<Event> event_queue;

    bool monster_move(Monster *entity);
    bool player_move(Player *entity);
    void move_to(Entity *entity, int row, int col);
    Coord get_target(Monster *entity);
    bool can_see(Coord a, Coord b);
    void new_floor();
    void update_player_view();
    public:
        Dungeon dungeon;
        View view;
        GameState(Dungeon dungeon);
        bool tick();
};


GameState init_state(Dungeon dungeon);
void destroy_state(GameState *state);
bool tick(GameState *state);
#endif
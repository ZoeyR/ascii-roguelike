#include <limits.h>
#include <stdio.h>

#include <loop.h>
#include <dungeon/entities.h>
#include <dungeon/dungeon.h>
#include <util/distance.h>
#include <io.h>
#include <collections/heap.h>

#include <ncurses.h>
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

static int _event_comparator(void* this, void* to);
static bool _monster_move(GameState *state, Entity *entity);
static bool _player_move(GameState *state, Entity *entity);
static void _move_to(GameState *state, Entity *entity, int row, int col);
static Coord _get_target(GameState *state, Entity *entity);
static bool _can_see(GameState *state, Coord a, Coord b);
static int _length_no_tunnel(void *context, Coordinate *this, Coordinate *to);
static int _length_tunnel(void *context, Coordinate *this, Coordinate *to);
static void _rebuild_state(GameState *state);
static void _update_player_view(GameState *state);
GameState init_state(Dungeon dungeon) {
    GameState state;
    Heap heap = init_heap(_event_comparator, sizeof(Event));
    View view;
    
    for(EIdx i = 1; i <= store_size(dungeon.store); i++) {
        heap_push(&heap, &(Event){.turn = 0, .entity_id = i, .event_type = MOVE});
    }

    // TODO build the first view
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            view.blocks[row][col] = (DungeonBlock){.type = ROCK, .hardness = 200, .region = 0, .immutable = false, .entity_id = 0};
        }
    }
    state = (GameState){.event_queue = heap, .dungeon = dungeon, .view = view};
    _update_player_view(&state);

    return state;
}

void destroy_state(GameState *state) {
    destroy_heap(&state->event_queue);
    destroy_dungeon(&state->dungeon);
}

static void _rebuild_state(GameState *state) {
    destroy_heap(&state->event_queue);
    rebuild_dungeon(&state->dungeon);
    *state = init_state(state->dungeon);
}

bool tick(GameState *state) {
    Event event = *(Event *)unwrap(heap_pop(&state->event_queue), 1);
    Entity *entity = unwrap(entity_retrieve(state->dungeon.store, event.entity_id), 1);

    if (!entity_alive(entity)) {
        return false;
    }
    
    bool rebuilt = false;
    if (!is_player(entity)) {
        rebuilt = _monster_move(state, entity);
    } else {
        rebuilt = _player_move(state, entity);
    }

    if (!rebuilt) {
        event.turn = event.turn + 1000/entity_speed(entity);
        heap_push(&state->event_queue, &event);
    } else {
        for(int row = 0; row < DUNGEON_HEIGHT; row++) {
            for(int col = 0; col < DUNGEON_WIDTH; col++) {
                state->view.blocks[row][col] = (DungeonBlock){.type = ROCK, .hardness = 200, .region = 0, .immutable = false, .entity_id = 0};
            }
        }
    }
    _update_player_view(state);

    return is_player(entity);
}

static int _event_comparator(void* this, void* to) {
    Event *_this = (Event *)this;
    Event *_to = (Event *)to;

    return _this->turn - _to->turn;
}

static bool _monster_move(GameState *state, Entity *entity) {
    Dungeon *dungeon = &state->dungeon;
    int col = entity_col(entity);
    int row = entity_row(entity);
    relative_array(1, entity_row(entity), entity_col(entity), DUNGEON_HEIGHT, DUNGEON_WIDTH, );
    int adjacent[8][2] = {{top, left}   , {top, col}   , {top, right},
                          {row, left}   ,                {row, right},
                          {bottom, left}, {bottom, col}, {bottom, right}};

    if (is_erratic(entity) && better_rand(1)) {
        int idx = better_rand(7);
        _move_to(state, entity, adjacent[idx][0], adjacent[idx][1]);
        return false;
    }

     // get the location this monster is moving to
    Coord target = _get_target(state, entity);
    if (is_smart(entity)) {
        //first get the correct distance map
        Distances distance_map;
        if (is_tunneling(entity)) {
            distance_map = dijkstra(dungeon, target.row, target.col, _length_tunnel);
        } else {
            distance_map = dijkstra(dungeon, target.row, target.col, _length_no_tunnel);
        }
        int lowest = 0;
        for(int i = 0; i < 8; i++) {
            int old_distance = distance_map.d[adjacent[lowest][0]][adjacent[lowest][1]];
            int distance = distance_map.d[adjacent[i][0]][adjacent[i][1]];
            if (distance < old_distance) {
                lowest = i;
            }
        }
        
        if (dungeon->blocks[adjacent[lowest][0]][adjacent[lowest][1]].type == ROCK ||
            dungeon->blocks[adjacent[lowest][0]][adjacent[lowest][1]].type == PILLAR) {
            
            uint8_t hardness = dungeon->blocks[adjacent[lowest][0]][adjacent[lowest][1]].hardness;
            hardness = hardness < 85 ? 0 : hardness - 85;
            dungeon->blocks[adjacent[lowest][0]][adjacent[lowest][1]].hardness = hardness;

            if (hardness == 0) {
                dungeon->blocks[adjacent[lowest][0]][adjacent[lowest][1]].type = HALL;
                _move_to(state, entity, adjacent[lowest][0], adjacent[lowest][1]);
            }
        } else {
            _move_to(state, entity, adjacent[lowest][0], adjacent[lowest][1]);
        }
    } else {
        int to_row = entity_row(entity) > target.row ? entity_row(entity) + 1 : entity_row(entity) - 1;
        int to_col = entity_col(entity) > target.col ? entity_col(entity) + 1 : entity_col(entity) - 1;
        if (entity_row(entity) == target.row) {
            to_row = entity_row(entity);
        }
        if (entity_col(entity) == target.col) {
            to_col = entity_col(entity);
        }
        _move_to(state, entity, to_row, to_col);
    }

    return false;
}

static bool _player_move(GameState *state, Entity *entity) {
    bool control_mode = true;
    bool debug = false;
    int col = entity_col(entity);
    int row = entity_row(entity);

    int view_col = col;
    int view_row = row;
    relative_array(1, entity_row(entity), entity_col(entity), DUNGEON_HEIGHT, DUNGEON_WIDTH, );

    // loop until a movement is actually made
    while (true) {
        int ch = get_input();
        if (control_mode) {
            switch (ch) {
                case 'y':
                case '7':
                    _move_to(state, entity, top, left);
                    return false;
                case 'k':
                case '8':
                     _move_to(state, entity, top, col);
                    return false;
                case 'u':
                case '9':
                     _move_to(state, entity, top, right);
                    return false;
                case 'l':
                case '6':
                     _move_to(state, entity, row, right);
                    return false;
                case 'n':
                case '3':
                     _move_to(state, entity, bottom, right);
                    return false;
                case 'j':
                case '2':
                     _move_to(state, entity, bottom, col);
                    return false;
                case 'b':
                case '1':
                     _move_to(state, entity, bottom, left);
                    return false;
                case 'h':
                case '4':
                     _move_to(state, entity, row, left);
                    return false;
                case '<':
                    if (state->dungeon.blocks[row][col].type == UPSTAIRS) {
                        _rebuild_state(state);
                        return true;
                    }
                case '>':
                    if (state->dungeon.blocks[row][col].type == DOWNSTAIRS) {
                        _rebuild_state(state);
                        return true;
                    }
                case ' ':
                case '5':
                    return false;
                case 'L':
                    control_mode = false;
                    break;
                case 'Q':
                    exit(0);
            }
        } else {
            switch (ch) {
                case 'k':
                case '8':
                    view_row--;
                    break;
                case 'l':
                case '6':
                    view_col++;
                    break;
                case 'j':
                case '2':
                    view_row++;
                    break;
                case 'h':
                case '4':
                    view_col--;
                    break;
                case 'D':
                    debug = !debug;
                    break;
                case 27:
                    control_mode = true;
                    view_row = row;
                    view_col = col;
                    break;
                case 'Q':
                    exit(0);
            }

            if (debug) {
                print_dungeon(&state->dungeon, view_row, view_col);
            } else {
                print_view(state, view_row, view_col);
            }
        }
    }
}

static Coord _get_target(GameState *state, Entity *entity) {
    Entity *player = unwrap(entity_retrieve(state->dungeon.store, state->dungeon.player_id), 1);


    if (is_telepathic(entity)) {
        return (Coord){.row = entity_row(player), .col = entity_col(player)};
    }

    // check for pc line of sight
    if (_can_see(state, 
                 (Coord){.row = entity_row(entity), .col = entity_col(entity)},
                 (Coord){.row = entity_row(player), .col = entity_col(player)})) {
        return (Coord){.row = entity_row(player), .col = entity_col(player)};
    } else if (is_smart(entity)) {
        return (Coord){.row = player_last_seen_row(entity), .col = player_last_seen_col(entity)};
    } else {
        return (Coord){.row = entity_row(entity), .col = entity_col(entity)};
    }
}

static bool _can_see(GameState *state, Coord a, Coord b) {
    Coord current = a;
    int d_row = abs(a.row - b.row);
    int d_col = -abs(a.col - b.col);
    int row_dir = a.row < b.row ? 1 : -1;
    int col_dir = a.col < b.col ? 1 : -1;
    int error = d_row + d_col;

    while(true) {
        if (state->dungeon.blocks[current.row][current.col].type == ROCK ||
            state->dungeon.blocks[current.row][current.col].type == PILLAR) {
            return false;
        }

        if (current.row == b.row && current.col == b.col) {
            return true;
        }

        int tmp_error = error * 2;
        if (tmp_error <= d_row) {
            error += d_row;
            current.col += col_dir;
        }

        if (tmp_error >= d_col) {
            error += d_col;
            current.row += row_dir;
        }
    }
}

static void _move_to(GameState *state, Entity *entity, int to_row, int to_col) {
    Dungeon *dungeon = &state->dungeon;
    if (dungeon->blocks[to_row][to_col].type == ROCK ||
        dungeon->blocks[to_row][to_col].type == PILLAR) {
            return;
    }
    int row = entity_row(entity);
    int col = entity_col(entity);
    dungeon->blocks[row][col].entity_id = 0;

    // check if we have killed something
    if (dungeon->blocks[to_row][to_col].entity_id != 0 && dungeon->blocks[to_row][to_col].entity_id != entity_index(entity)) {
        Entity *entity = unwrap(entity_retrieve(dungeon->store, dungeon->blocks[to_row][to_col].entity_id), 1);
        kill_entity(entity);
        if (!is_player(entity)) {
            dungeon->monster_count--;
        }
    }
    dungeon->blocks[to_row][to_col].entity_id = entity_index(entity);

    set_entity_row(entity, to_row);
    set_entity_col(entity, to_col);
}

static int _length_no_tunnel(void *context, Coordinate *this, Coordinate *to) {
    Dungeon *dungeon = (Dungeon *)context;
    (void)(this);
    DungeonBlock b_to = dungeon->blocks[to->row][to->col];
    if (b_to.type == ROCK || b_to.type == PILLAR) {
        return INT_MAX;
    } else {
        return 1;
    }
}

static int _length_tunnel(void *context, Coordinate *this, Coordinate *to) {
    Dungeon *dungeon = (Dungeon *)context;
    (void)(this);
    DungeonBlock b_to = dungeon->blocks[to->row][to->col];
    if (b_to.type == ROCK || b_to.type == PILLAR) {
        if(b_to.immutable) {
            return INT_MAX;
        }

        uint8_t hardness = b_to.hardness;
        //printf("hardness %d\n", hardness);
        if (hardness < HARDNESS_TIER_1) {
            return 1;
        } else if (hardness < HARDNESS_TIER_2) {
            return 2;
        } else if (hardness < HARDNESS_TIER_3) {
            return 3;
        } else if (hardness < HARDNESS_TIER_MAX) {
            return 4;
        } else {
            return INT_MAX;
        }
    } else {
        return 1;
    }
}

static void _update_player_view(GameState *state) {
    Entity *player = unwrap(entity_retrieve(state->dungeon.store, state->dungeon.player_id), 1);
    int p_row = entity_row(player);
    int p_col = entity_col(player);

    int l_row_bound = p_row - 4 < 0 ? 0 : p_row - 4;
    int u_row_bound = p_row + 4 > DUNGEON_HEIGHT ? DUNGEON_HEIGHT : p_row + 4;

    int l_col_bound = p_col - 4 < 0 ? 0 : p_col - 4;
    int u_col_bound = p_col + 4 > DUNGEON_WIDTH ? DUNGEON_WIDTH : p_col + 4;

    // clear out old entities
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            state->view.blocks[row][col].entity_id = 0;
        }
    }

    for(int row = l_row_bound; row < u_row_bound; row++) {
        for(int col = l_col_bound; col < u_col_bound; col++) {
            state->view.blocks[row][col] = state->dungeon.blocks[row][col];
        }
    }
}
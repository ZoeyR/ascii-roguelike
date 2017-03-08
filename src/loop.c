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
static void _monster_move(Dungeon *dungeon, Entity *entity);
static void _player_move(Dungeon *dungeon, Entity *entity);
static void _move_to(Dungeon *dungeon, Entity *entity, int row, int col);
static Coord _get_target(Dungeon *dungeon, Entity *entity);
static bool _can_see(Dungeon *dungeon, Coord a, Coord b);
static int _length_no_tunnel(void *context, Coordinate *this, Coordinate *to);
static int _length_tunnel(void *context, Coordinate *this, Coordinate *to);

GameState init_state(Dungeon *dungeon) {
    Heap heap = init_heap(_event_comparator, sizeof(Event));

    for(EIdx i = 1; i <= dungeon->store.list.size; i++) {
        heap_push(&heap, &(Event){.turn = 0, .entity_id = i, .event_type = MOVE});
    }

    return (GameState){.event_queue = heap, .player_dead = false};
}

bool tick(Dungeon *dungeon, GameState *state) {
    Event event = *(Event *)unwrap(heap_pop(&state->event_queue), 1);
    Entity *entity = unwrap(entity_retrieve(&dungeon->store, event.entity_id), 1);

    if (!entity->alive) {
        return false;
    }
    
    if (entity->type == MONSTER) {
        _monster_move(dungeon, entity);
    } else if (entity->type == PLAYER) {
        _player_move(dungeon, entity);
    }
    event.turn = event.turn + 1000/entity->speed;
    heap_push(&state->event_queue, &event);

    return entity->type == PLAYER;
}

static int _event_comparator(void* this, void* to) {
    Event *_this = (Event *)this;
    Event *_to = (Event *)to;

    return _this->turn - _to->turn;
}

static void _monster_move(Dungeon *dungeon, Entity *entity) {
    int col = entity->monster.col;
    int row = entity->monster.row;
    relative_array(1, entity->monster.row, entity->monster.col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );
    int adjacent[8][2] = {{top, left}   , {top, col}   , {top, right},
                          {row, left}   ,                {row, right},
                          {bottom, left}, {bottom, col}, {bottom, right}};
    if (entity->monster.erratic && better_rand(1)) {
        int idx = better_rand(7);
        _move_to(dungeon, entity, adjacent[idx][0], adjacent[idx][1]);
        return;
    }

     // get the location this monster is moving to
    Coord target = _get_target(dungeon, entity);

    if (entity->monster.smart) {
        //first get the correct distance map
        Distances distance_map;
        if (entity->monster.tunneling) {
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
                _move_to(dungeon, entity, adjacent[lowest][0], adjacent[lowest][1]);
            }
        } else {
            _move_to(dungeon, entity, adjacent[lowest][0], adjacent[lowest][1]);
        }
    } else {
        int to_row = entity->monster.row > target.row ? entity->monster.row + 1 : entity->monster.row - 1;
        int to_col = entity->monster.col > target.col ? entity->monster.col + 1 : entity->monster.col - 1;
        if (entity->monster.row == target.row) {
            to_row = entity->monster.row;
        }
        if (entity->monster.col == target.col) {
            to_col = entity->monster.col;
        }
        _move_to(dungeon, entity, to_row, to_col);
    }
}

static void _player_move(Dungeon *dungeon, Entity *entity) {
    bool control_mode = true;
    int col = entity->player.col;
    int row = entity->player.row;

    int view_col = col;
    int view_row = row;
    relative_array(1, entity->player.row, entity->player.col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );

    // loop until a movement is actually made
    while (true) {
        int ch = get_input();
        if (control_mode) {
            switch (ch) {
                case 'y':
                case '7':
                    _move_to(dungeon, entity, top, left);
                    return;
                case 'k':
                case '8':
                     _move_to(dungeon, entity, top, col);
                    return;
                case 'u':
                case '9':
                     _move_to(dungeon, entity, top, right);
                    return;
                case 'l':
                case '6':
                     _move_to(dungeon, entity, row, right);
                    return;
                case 'n':
                case '3':
                     _move_to(dungeon, entity, bottom, right);
                    return;
                case 'j':
                case '2':
                     _move_to(dungeon, entity, bottom, col);
                    return;
                case 'b':
                case '1':
                     _move_to(dungeon, entity, bottom, left);
                    return;
                case 'h':
                case '4':
                     _move_to(dungeon, entity, row, left);
                    return;
                case '<':
                     _move_to(dungeon, entity, row, left);
                    return;
                case '>':
                     _move_to(dungeon, entity, row, left);
                    return;
                case ' ':
                case '5':
                    return;
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
                    print_dungeon(dungeon, --view_row, view_col);
                    break;
                case 'l':
                case '6':
                    print_dungeon(dungeon, view_row, ++view_col);
                    break;
                case 'j':
                case '2':
                    print_dungeon(dungeon, ++view_row, view_col);
                    break;
                case 'h':
                case '4':
                    print_dungeon(dungeon, view_row, --view_col);
                    break;
                case 27:
                    control_mode = true;
                    view_row = row;
                    view_col = col;
                    print_dungeon(dungeon, row, col);
                    break;
                case 'Q':
                    exit(0);
            }
        }
    }
}

static Coord _get_target(Dungeon *dungeon, Entity *entity) {
    Entity *player = unwrap(entity_retrieve(&dungeon->store, dungeon->player_id), 1);

    if (entity->monster.telepathic) {
        return (Coord){.row = player->player.row, .col = player->player.col};
    }

    // check for pc line of sight
    if (_can_see(dungeon, 
                 (Coord){.row = entity->monster.row, .col = entity->monster.col},
                 (Coord){.row = player->player.row, .col = player->player.col})) {
        return (Coord){.row = player->player.row, .col = player->player.col};
    } else if (entity->monster.smart) {
        return (Coord){.row = entity->monster.pc_last_seen[0], .col = entity->monster.pc_last_seen[1]};
    } else {
        return (Coord){.row = entity->monster.row, .col = entity->monster.col};
    }
}

static bool _can_see(Dungeon *dungeon, Coord a, Coord b) {
    Coord current = a;
    int d_row = abs(a.row - b.row);
    int d_col = -abs(a.col - b.col);
    int row_dir = a.row < b.row ? 1 : -1;
    int col_dir = a.col < b.col ? 1 : -1;
    int error = d_row + d_col;

    while(true) {
        if (dungeon->blocks[current.row][current.col].type == ROCK ||
            dungeon->blocks[current.row][current.col].type == PILLAR) {
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

static void _move_to(Dungeon *dungeon, Entity *entity, int to_row, int to_col) {
    if (dungeon->blocks[to_row][to_col].type == ROCK ||
        dungeon->blocks[to_row][to_col].type == PILLAR) {
            return;
    }
    int row = entity->type == MONSTER ? entity->monster.row : entity->player.row;
    int col = entity->type == MONSTER ? entity->monster.col : entity->player.col;
    dungeon->blocks[row][col].entity_id = 0;

    // check if we have killed something
    if (dungeon->blocks[to_row][to_col].entity_id != 0 && dungeon->blocks[to_row][to_col].entity_id != entity->index) {
        Entity *entity = unwrap(entity_retrieve(&dungeon->store, dungeon->blocks[to_row][to_col].entity_id), 1);
        entity->alive = false;
        if (entity->type == MONSTER) {
            dungeon->monster_count--;
        }
    }
    dungeon->blocks[to_row][to_col].entity_id = entity->index;
    if (entity->type == MONSTER) {
        entity->monster.row = to_row;
        entity->monster.col = to_col;
    } else {
        entity->player.row = to_row;
        entity->player.col = to_col;
    }
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
#include <limits.h>
#include <stdio.h>

#include <loop.h>
#include <dungeon/entities.h>
#include <dungeon/dungeon.h>
#include <util/distance.h>
#include <io.h>
#include <collections/heap.h>

#include <ncurses.h>

static void _init_floor_state(Dungeon &dungeon, Heap<Event> &heap, View &view);
static int _length_no_tunnel(void *context, Coordinate *from, Coordinate *to);
static int _length_tunnel(void *context, Coordinate *from, Coordinate *to);

static void _init_floor_state(Dungeon &dungeon, Heap<Event> &heap, View &view) {
    for(EIdx i = 1; i <= dungeon.store->size(); i++) {
        heap.push((Event){.turn = 0, .entity_id = i, .event_type = Event::MOVE});
    }

    // TODO build the first view
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            view.blocks[row][col] = (DungeonBlock){.type = DungeonBlock::ROCK, .hardness = 200, .region = 0, .immutable = false, .entity_id = 0};
        }
    }
}

GameState::GameState(Dungeon dungeon): event_queue([] (auto from, auto to) {return from.turn - to.turn;}) {
    this->dungeon = dungeon;
    _init_floor_state(dungeon, this->event_queue, this->view);
    
    update_player_view();
}

void destroy_state(GameState *state) {
    destroy_dungeon(&state->dungeon);
}

void GameState::new_floor() {
    event_queue.clear();
    rebuild_dungeon(&dungeon);
    _init_floor_state(dungeon, event_queue, view);
}

bool GameState::tick() {
    Event event = event_queue.pop();
    Entity *entity = dungeon.store->get(event.entity_id).unwrap();

    if (!entity->alive) {
        return false;
    }
    
    bool rebuilt = false;
    if (!is_player(entity)) {
        rebuilt = monster_move(static_cast<Monster *>(entity));
    } else {
        rebuilt = player_move(static_cast<Player *>(entity));
    }

    if (!rebuilt) {
        event.turn = event.turn + 1000/entity->speed;
        event_queue.push(event);
    } else {
        for(int row = 0; row < DUNGEON_HEIGHT; row++) {
            for(int col = 0; col < DUNGEON_WIDTH; col++) {
                view.blocks[row][col] = (DungeonBlock){.type = DungeonBlock::ROCK, .hardness = 200, .region = 0, .immutable = false, .entity_id = 0};
            }
        }
    }
    update_player_view();

    return is_player(entity);
}

bool GameState::monster_move(Monster *entity) {
    int col = entity->col;
    int row = entity->row;
    relative_array(1, entity->row, entity->col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );
    int adjacent[8][2] = {{top, left}   , {top, col}   , {top, right},
                          {row, left}   ,                {row, right},
                          {bottom, left}, {bottom, col}, {bottom, right}};

    if (entity->erratic && better_rand(1)) {
        int idx = better_rand(7);
        move_to(entity, adjacent[idx][0], adjacent[idx][1]);
        return false;
    }

     // get the location this monster is moving to
    Coord target = get_target(entity);
    if (entity->smart) {
        //first get the correct distance map
        Distances distance_map;
        if (entity->tunneling) {
            distance_map = dijkstra(&dungeon, target.row, target.col, _length_tunnel);
        } else {
            distance_map = dijkstra(&dungeon, target.row, target.col, _length_no_tunnel);
        }
        int lowest = 0;
        for(int i = 0; i < 8; i++) {
            int old_distance = distance_map.d[adjacent[lowest][0]][adjacent[lowest][1]];
            int distance = distance_map.d[adjacent[i][0]][adjacent[i][1]];
            if (distance < old_distance) {
                lowest = i;
            }
        }
        
        if (dungeon.blocks[adjacent[lowest][0]][adjacent[lowest][1]].type == DungeonBlock::ROCK ||
            dungeon.blocks[adjacent[lowest][0]][adjacent[lowest][1]].type == DungeonBlock::PILLAR) {
            
            uint8_t hardness = dungeon.blocks[adjacent[lowest][0]][adjacent[lowest][1]].hardness;
            hardness = hardness < 85 ? 0 : hardness - 85;
            dungeon.blocks[adjacent[lowest][0]][adjacent[lowest][1]].hardness = hardness;

            if (hardness == 0) {
                dungeon.blocks[adjacent[lowest][0]][adjacent[lowest][1]].type = DungeonBlock::HALL;
                move_to(entity, adjacent[lowest][0], adjacent[lowest][1]);
            }
        } else {
            move_to(entity, adjacent[lowest][0], adjacent[lowest][1]);
        }
    } else {
        int to_row = entity->row > target.row ? entity->row + 1 : entity->row - 1;
        int to_col = entity->col > target.col ? entity->col + 1 : entity->col - 1;
        if (entity->row == target.row) {
            to_row = entity->row;
        }
        if (entity->col == target.col) {
            to_col = entity->col;
        }
        move_to(entity, to_row, to_col);
    }

    return false;
}

bool GameState::player_move(Player *entity) {
    bool control_mode = true;
    bool debug = false;
    int col = entity->col;
    int row = entity->row;

    int view_col = col;
    int view_row = row;
    relative_array(1, entity->row, entity->col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );

    // loop until a movement is actually made
    while (true) {
        int ch = get_input();
        if (control_mode) {
            switch (ch) {
                case 'y':
                case '7':
                    move_to(entity, top, left);
                    return false;
                case 'k':
                case '8':
                     move_to(entity, top, col);
                    return false;
                case 'u':
                case '9':
                     move_to(entity, top, right);
                    return false;
                case 'l':
                case '6':
                     move_to(entity, row, right);
                    return false;
                case 'n':
                case '3':
                     move_to(entity, bottom, right);
                    return false;
                case 'j':
                case '2':
                     move_to(entity, bottom, col);
                    return false;
                case 'b':
                case '1':
                     move_to(entity, bottom, left);
                    return false;
                case 'h':
                case '4':
                     move_to(entity, row, left);
                    return false;
                case '<':
                    if (dungeon.blocks[row][col].type == DungeonBlock::UPSTAIRS) {
                        new_floor();
                        return true;
                    }
                case '>':
                    if (dungeon.blocks[row][col].type == DungeonBlock::DOWNSTAIRS) {
                        new_floor();
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
                print_dungeon(&dungeon, view_row, view_col);
            } else {
                print_view(this, view_row, view_col);
            }
        }
    }
}

Coord GameState::get_target(Monster *entity) {
    Entity *player = dungeon.store->get(dungeon.player_id).unwrap();


    if (entity->telepathic) {
        return (Coord){.row = player->row, .col = player->col};
    }

    // check for pc line of sight
    if (can_see((Coord){.row = entity->row, .col = entity->col},
                (Coord){.row = player->row, .col = player->col})) {
        return (Coord){.row = player->row, .col = player->col};
    } else if (entity->smart) {
        return (Coord){.row = entity->pc_last_seen[0], .col = entity->pc_last_seen[1]};
    } else {
        return (Coord){.row = entity->row, .col = entity->col};
    }
}

bool GameState::can_see(Coord a, Coord b) {
    Coord current = a;
    int d_row = abs(a.row - b.row);
    int d_col = -abs(a.col - b.col);
    int row_dir = a.row < b.row ? 1 : -1;
    int col_dir = a.col < b.col ? 1 : -1;
    int error = d_row + d_col;

    while(true) {
        if (dungeon.blocks[current.row][current.col].type == DungeonBlock::ROCK ||
            dungeon.blocks[current.row][current.col].type == DungeonBlock::PILLAR) {
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

void GameState::move_to(Entity *entity, int to_row, int to_col) {
    if (dungeon.blocks[to_row][to_col].type == DungeonBlock::ROCK ||
        dungeon.blocks[to_row][to_col].type == DungeonBlock::PILLAR) {
            return;
    }
    int row = entity->row;
    int col = entity->col;
    dungeon.blocks[row][col].entity_id = 0;

    // check if we have killed something
    if (dungeon.blocks[to_row][to_col].entity_id != 0 && dungeon.blocks[to_row][to_col].entity_id != entity->index) {
        Entity *entity = dungeon.store->get(dungeon.blocks[to_row][to_col].entity_id).unwrap();
        entity->alive = false;
        if (!is_player(entity)) {
            dungeon.monster_count--;
        }
    }
    dungeon.blocks[to_row][to_col].entity_id = entity->index;

    entity->row = to_row;
    entity->col = to_col;
}

static int _length_no_tunnel(void *context, Coordinate *from, Coordinate *to) {
    Dungeon *dungeon = (Dungeon *)context;
    (void)(from);
    DungeonBlock b_to = dungeon->blocks[to->row][to->col];
    if (b_to.type == DungeonBlock::ROCK || b_to.type == DungeonBlock::PILLAR) {
        return INT_MAX;
    } else {
        return 1;
    }
}

static int _length_tunnel(void *context, Coordinate *from, Coordinate *to) {
    Dungeon *dungeon = (Dungeon *)context;
    (void)(from);
    DungeonBlock b_to = dungeon->blocks[to->row][to->col];
    if (b_to.type == DungeonBlock::ROCK || b_to.type == DungeonBlock::PILLAR) {
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

void GameState::update_player_view() {
    Entity *player = dungeon.store->get(dungeon.player_id).unwrap();
    int p_row = player->row;
    int p_col = player->col;

    int l_row_bound = p_row - 4 < 0 ? 0 : p_row - 4;
    int u_row_bound = p_row + 5 > DUNGEON_HEIGHT ? DUNGEON_HEIGHT : p_row + 5;

    int l_col_bound = p_col - 4 < 0 ? 0 : p_col - 4;
    int u_col_bound = p_col + 5 > DUNGEON_WIDTH ? DUNGEON_WIDTH : p_col + 5;

    // clear out old entities
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            view.blocks[row][col].entity_id = 0;
        }
    }

    for(int row = l_row_bound; row < u_row_bound; row++) {
        for(int col = l_col_bound; col < u_col_bound; col++) {
            view.blocks[row][col] = dungeon.blocks[row][col];
        }
    }
}
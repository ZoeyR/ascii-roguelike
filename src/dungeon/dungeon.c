#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>

#include <dungeon/dungeon.h>
#include <io.h>
#include <util/util.h>
#include <util/list.h>

static void _generate_veins(Dungeon *dungeon, int hardness, int liklihood);
static void _generate_maze(Dungeon *dungeon, int windiness, int max_maze_size);
static bool _can_place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row);
static void _place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row);
static void _fill_maze(Dungeon *dungeon);
static void _unfreeze_rooms(Dungeon *dungeon);

Dungeon create_dungeon(int room_tries, int min_rooms, int hardness, int windiness, int max_maze_size, int imperfection_chance) {
    Dungeon dungeon;
    dungeon.regions = 0;
    // fill dungeon with random noise
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            char hardness;
            bool immutable;
            if (col == 0 || col == DUNGEON_WIDTH - 1 || 
                    row == 0 || row == DUNGEON_HEIGHT - 1) {
                hardness = 255;
                immutable = true;
            } else {
                hardness = better_rand(1) + 2;
                immutable = false;
            }

            DungeonBlock block = {.type = ROCK, .hardness = hardness, .region = 0, .immutable = immutable};
            dungeon.blocks[row][col] = block;
        }
    }

    // create veins of hard and soft rock
    _generate_veins(&dungeon, hardness, 300);

    // generate enough rooms for the dungeon to be reasonably filled
    int rooms = 0;
    int tries = 0;
    while(rooms < min_rooms || tries < room_tries) {
        tries++;

        // rooms must be odd sized for the generation algorithm to work
        int width = (better_rand(7) * 2) + 13;
        double ratio = (better_rand(50) / 100.0) + 0.5;
        int height;
        if(better_rand(1) == 1) {
            height = width * ratio < 9?9:width * ratio;
        } else {
            height = width / ratio > 27?27:width / ratio;
        }

        if (height % 2 == 0) {
            height++;
        }
        
        DungeonRoom room = create_room(width, height);
        int col = (better_rand((DUNGEON_WIDTH - width) / 2) * 2);
        int row = (better_rand((DUNGEON_HEIGHT - height) / 2) * 2);

        if (!_can_place_room(&dungeon, &room, col, row)) {
            continue;
        } else {
            _place_room(&dungeon, &room, col, row);
            rooms++;
        }
    }

    // generate maze
    printf("generate maze\n");
    _generate_maze(&dungeon, windiness, max_maze_size);

    // since the maze is now generated rooms can be unfrozen
    printf("unfreeze_rooms\n");
    _unfreeze_rooms(&dungeon);

    merge_regions(&dungeon, imperfection_chance);

    _fill_maze(&dungeon);
    return dungeon;
}

static void _create_vein(Dungeon *dungeon, int hardness, int row, int col) {
    if (row % 2 != 0 || col % 2 != 0) {
        return;
    }

    int last_direction = better_rand(3);

    while(better_rand(99) < 98) {
        if (dungeon->blocks[row][col].immutable) {
            break;
        }
        dungeon->blocks[row][col].hardness = hardness;

        relative_array(1, row, col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );
        relative_array(2, row, col, DUNGEON_HEIGHT, DUNGEON_WIDTH, _extra);

        int adjacent[4][2][2] = {{{top, col}, {top_extra, col}},
                                 {{row, right}, {row, right_extra}},
                                 {{bottom, col}, {bottom_extra, col}},
                                 {{row, left}, {row, left_extra}}};
        
        bool can_grow[4] = {false, false, false, false};
        for (int i = 0; i < 4; i++) {
            DungeonBlock block_a = dungeon->blocks[adjacent[i][0][0]][adjacent[i][0][1]];
            DungeonBlock block_b = dungeon->blocks[adjacent[i][1][0]][adjacent[i][1][1]];
            if(!block_a.immutable && !block_b.immutable) {
                can_grow[i] = true;
            }
        }

        int next_dir;
        if (better_rand(100) > 10) {
            next_dir = last_direction;
        } else {
            do {
                next_dir = better_rand(3);
            } while(!can_grow[next_dir]);
        }

        int row_a = adjacent[next_dir][0][0];
        int col_a = adjacent[next_dir][0][1];
        int row_b = adjacent[next_dir][1][0];
        int col_b = adjacent[next_dir][1][1];

        dungeon->blocks[row_a][col_a].hardness = hardness;
        dungeon->blocks[row_b][col_b].hardness = hardness;
        row = row_b;
        col = col_b;
        last_direction = next_dir;
    }
}

static void _generate_veins(Dungeon *dungeon, int hardness, int likelihood) {
    
    for(int row = 2; row < DUNGEON_HEIGHT; row += 2) {
        for(int col = 2; col < DUNGEON_WIDTH; col += 2) {
            if (dungeon->blocks[row][col].immutable || better_rand(likelihood) != 0) {
                continue;
            }

            if (better_rand(99) < hardness) {
                _create_vein(dungeon, 4, row, col);
            } else {
                _create_vein(dungeon, 1, row, col);
            }
        }
    }
}

static bool _can_place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row) {
    int dungeon_row_start = row;
    int dungeon_col_start = col;    
    int dungeon_row_end = row + room->height;
    int dungeon_col_end = col + room->width;

    if (dungeon_row_end > DUNGEON_HEIGHT || dungeon_col_end > DUNGEON_WIDTH) {
        return false;
    }

    for (row = dungeon_row_start; row < dungeon_row_end; row++) {
        for (col = dungeon_col_start; col < dungeon_col_end; col++) {
            int room_row = row - dungeon_row_start + room->start_row;
            int room_col = col - dungeon_col_start + room->start_col;
            if (dungeon->blocks[row][col].type != ROCK || dungeon->blocks[row][col].immutable || 
            dungeon->blocks[row][col].hardness > 3 || dungeon->blocks[row][col].hardness < 2) {
                return false;
            }

            if (room->blocks[room_row][room_col].type == ROCK) {
                continue;
            }

            if (col == dungeon_col_start || col == dungeon_col_end - 1
                || row == dungeon_row_start || row == dungeon_row_end) {
                    continue;
            }
        }
    }

    return true;
}

static void _place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row) {
    int dungeon_row_start = row;
    int dungeon_col_start = col;
    int dungeon_row_end = row + room->height;
    int dungeon_col_end = col + room->width;

    if (dungeon_row_end > DUNGEON_HEIGHT || dungeon_col_end > DUNGEON_WIDTH) {
        return;
    }

    dungeon->regions++;
    for (row = dungeon_row_start; row < dungeon_row_end; row++) {
        for (col = dungeon_col_start; col < dungeon_col_end; col++) {
            int room_row = row - dungeon_row_start + room->start_row;
            int room_col = col - dungeon_col_start + room->start_col;
            dungeon->blocks[row][col].type = room->blocks[room_row][room_col].type;
            dungeon->blocks[row][col].region = dungeon->regions;

            // room blocks are immutable to prevent mazes pathing into their area
            if (row != dungeon_row_start && row != dungeon_row_end
                && col != dungeon_col_start && col != dungeon_col_end) {
                dungeon->blocks[row][col].immutable = true;
            }  
        }
    }
}

static void _generate_maze(Dungeon *dungeon, int windiness, int max_maze_size) {
    for (int row = 1; row < DUNGEON_HEIGHT; row += 2) {
        for (int col = 1; col < DUNGEON_WIDTH; col += 2) {
            if (dungeon->blocks[row][col].type != ROCK || dungeon->blocks[row][col].immutable || 
            dungeon->blocks[row][col].hardness > 3 || dungeon->blocks[row][col].hardness < 2) {
                continue;
            }

            dungeon->regions++;
            // carve this section of the maze
            List carved_list = init_list(sizeof(int));
            dungeon->blocks[row][col].type = HALL;
            dungeon->blocks[row][col].region = dungeon->regions;

            int to_push = (row * DUNGEON_WIDTH) + col;
            list_push(&carved_list, (void *)&to_push);

            int maze_size = 0;
            while(carved_list.size > 0 && maze_size++ < max_maze_size) {
                list_shuffle(&carved_list);
                int coord = *(int *)list_pop(&carved_list);

                // decompose the coordinate
                int col = coord % DUNGEON_WIDTH;
                int row = coord / DUNGEON_WIDTH;

                // print_dungeon(dungeon);
                // printf("\nrow:%d col:%d\n", row, col);
                // scanf("%s", (char[]){' ', ' ', ' '});

                relative_array(1, row, col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );
                relative_array(2, row, col, DUNGEON_HEIGHT, DUNGEON_WIDTH, _extra);

                int adjacent[4][2][2] = {{{top, col}, {top_extra, col}},
                                         {{row, right}, {row, right_extra}},
                                         {{bottom, col}, {bottom_extra, col}},
                                         {{row, left}, {row, left_extra}}};

                int carved_index = 0;
                int total_carved = 0;
                for (int i = 0; i < 4; i++) {
                    DungeonBlock block_a = dungeon->blocks[adjacent[i][0][0]][adjacent[i][0][1]];
                    DungeonBlock block_b = dungeon->blocks[adjacent[i][1][0]][adjacent[i][1][1]];
                    if(block_a.type != ROCK && block_b.type != ROCK) {
                        total_carved++;
                        carved_index = i;
                    }
                }

                // calculate the last direction
                int last_dir;
                if (total_carved == 1) {
                    last_dir = (carved_index + 2) % 4;
                } else {
                    last_dir = better_rand(3);
                }

                int lowest_weight = INT_MAX;
                int lowest_weight_index = -1;
                bool can_carve[4] = {false, false, false, false};
                for (int i = 0; i < 4; i++) {
                    DungeonBlock block_a = dungeon->blocks[adjacent[i][0][0]][adjacent[i][0][1]];
                    DungeonBlock block_b = dungeon->blocks[adjacent[i][1][0]][adjacent[i][1][1]];
                    if((!block_a.immutable && block_a.type == ROCK && block_a.hardness < 4 && block_a.hardness > 1) && 
                    (!block_b.immutable && block_b.type == ROCK && block_b.hardness < 4 && block_a.hardness > 1)) {
                        can_carve[i] = true;
                        int weight = block_a.hardness + block_b.hardness;
                        if (i == last_dir) {
                            weight -= 1;;
                        }

                        if (weight <= lowest_weight) {
                            lowest_weight = weight;
                            lowest_weight_index = i;
                        }
                    }
                }
                // printf("0:%d 1:%d 2:%d 3:%d\n", can_carve[0], can_carve[1], can_carve[2], can_carve[3]);
                // printf("ind:%d\n", lowest_weight_index);
                if (lowest_weight_index == -1) {
                    continue;
                }

                int next_dir;
                if (better_rand(99) + 1 > windiness && can_carve[lowest_weight_index]) {
                    next_dir = lowest_weight_index;
                } else {
                    do {
                        next_dir = better_rand(3);
                    } while(!can_carve[next_dir]);
                }

                int row_a = adjacent[next_dir][0][0];
                int col_a = adjacent[next_dir][0][1];
                int row_b = adjacent[next_dir][1][0];
                int col_b = adjacent[next_dir][1][1];

                dungeon->blocks[row_a][col_a].type = HALL;
                dungeon->blocks[row_b][col_b].type = HALL;
                dungeon->blocks[row_a][col_a].region = dungeon->regions;
                dungeon->blocks[row_b][col_b].region = dungeon->regions;

                list_push(&carved_list, &coord);
                int coord_b = (row_b * DUNGEON_WIDTH) + col_b;
                list_push(&carved_list, &coord_b);
            }

            destroy_list(&carved_list);
        }
    }
}

static void _fill_maze_helper(Dungeon *dungeon, int row, int col) {
    int right = (col >= DUNGEON_WIDTH - 1) ? col : col + 1;
    int left = (col <= 0) ? 0 : col - 1;
    int top = (row <= 0) ? 0 : row - 1;
    int bottom = (row >= DUNGEON_HEIGHT - 1) ? row : row + 1;

    int total_open = 0;
    int last_open_col = 0;
    int last_open_row = 0;
    int adjacent[4][2] = {{top, col},
                                {row, right},
                                {bottom, col},
                                {row, left}};
    
    for (int i = 0; i < 4; i++) {
        if (dungeon->blocks[adjacent[i][0]][adjacent[i][1]].type != ROCK) {
            total_open++;
            last_open_row = adjacent[i][0];
            last_open_col = adjacent[i][1];
        }
    }

    if (total_open == 1) {
        dungeon->blocks[row][col].type = ROCK;
        _fill_maze_helper(dungeon, last_open_row, last_open_col);
    }

    if (total_open == 0) {
         dungeon->blocks[row][col].type = ROCK;
    }
}

static void _fill_maze(Dungeon *dungeon) {
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            _fill_maze_helper(dungeon, row, col);
        }
    }
}

static void _unfreeze_rooms(Dungeon *dungeon) {
    for(int row = 1; row < DUNGEON_HEIGHT - 1; row++) {
        for(int col = 1; col < DUNGEON_WIDTH - 1; col++) {
            dungeon->blocks[row][col].immutable = false;
        }
    }
}
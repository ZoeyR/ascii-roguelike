#include "dungeon.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "util.h"
#include "output.h"
#include <limits.h>
#include <stdio.h>

#define DUNGEON_GEN_WIDTH 159
#define DUNGEON_GEN_HEIGHT 105

static void generate_veins(Dungeon *dungeon, int hardness);
static void generate_maze(Dungeon *dungeon, int windiness);
static bool can_place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row);
static void place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row);

Dungeon create_dungeon(int room_tries, int min_rooms, int hardness) {
    Dungeon dungeon;
    // fill dungeon with random noise
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            char hardness;
            if (col == 0 || col == DUNGEON_WIDTH - 1 || 
                    row == 0 || row == DUNGEON_HEIGHT - 1) {
                hardness = 4;
            } else {
                hardness = (rand() % 2) + 1;
            }

            DungeonBlock block = {.type = ROCK, .hardness = hardness };
            dungeon.blocks[row][col] = block;
        }
    }

    // create veins of hard and soft rock
    generate_veins(&dungeon, hardness);

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
        int col = (better_rand((DUNGEON_GEN_WIDTH - width) / 2) * 2);
        int row = (better_rand((DUNGEON_GEN_HEIGHT - height) / 2) * 2);

        if (!can_place_room(&dungeon, &room, col, row)) {
            continue;
        } else {
            place_room(&dungeon, &room, col, row);
            rooms++;
        }
    }

    // generate maze
    generate_maze(&dungeon, 20);
    printf("end generation\n");
    return dungeon;
}

static void create_vein(Dungeon *dungeon, int hardness) {
    int row = rand() % DUNGEON_HEIGHT;
    int col = rand() % DUNGEON_WIDTH;

    int last_direction = rand() % 4;

    while(rand() % 100 < 98) {
        dungeon->blocks[row][col].hardness = hardness;

        // choose next position
        if (rand() % 100 < 10) {
            last_direction = rand() % 4;
        }

        switch(last_direction) {
            default:
            case 0:
                row--;
                break;
            case 1:
                col++;
                break;
            case 2:
                row++;
                break;
            case 3:
                col--;
                break;
        }

        if(row >= DUNGEON_HEIGHT - 1 || row <= 0 || col >= DUNGEON_WIDTH - 1 || col <= 0) {
            break;
        }
    }
}

static void generate_veins(Dungeon *dungeon, int hardness) {
    
    // generate hard veins
    while(rand() % 100 < hardness) {
        create_vein(dungeon, 3);
    }

    // generate soft veins
    while(rand() % 100 > hardness) {
        create_vein(dungeon, 0);
    }
}

static bool can_place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row) {
    int dungeon_row_start = row;
    int dungeon_col_start = col;    
    int dungeon_row_end = row + room->height;
    int dungeon_col_end = col + room->width;

    if (dungeon_row_end >= DUNGEON_GEN_HEIGHT || dungeon_col_end >= DUNGEON_GEN_WIDTH) {
        return false;
    }

    for (row = dungeon_row_start; row < dungeon_row_end; row++) {
        for (col = dungeon_col_start; col < dungeon_col_end; col++) {
            if (dungeon->blocks[row][col].type != ROCK) {
                return false;
            }

            if (col == dungeon_col_start || col == dungeon_col_end - 1
                || row == dungeon_row_start || row == dungeon_row_end) {
                    continue;
            }

            if(dungeon->blocks[row][col].hardness > 2 || dungeon->blocks[row][col].hardness < 1) {
                return false;
            }
        }
    }

    return true;
}

static void place_room(Dungeon *dungeon, DungeonRoom *room, int col, int row) {
    int dungeon_row_start = row;
    int dungeon_col_start = col;
    int dungeon_row_end = row + room->height;
    int dungeon_col_end = col + room->width;

    if (dungeon_row_end >= DUNGEON_GEN_HEIGHT || dungeon_col_end >= DUNGEON_GEN_WIDTH) {
        return;
    }

    for (row = dungeon_row_start; row < dungeon_row_end; row++) {
        for (col = dungeon_col_start; col < dungeon_col_end; col++) {
            int room_row = row - dungeon_row_start + room->start_row;
            int room_col = col - dungeon_col_start + room->start_col;
            dungeon->blocks[row][col].type = room->blocks[room_row][room_col].type;
        }
    }
}

static void generate_maze(Dungeon *dungeon, int windiness) {
    for (int row = 1; row < DUNGEON_GEN_HEIGHT; row += 2) {
        for (int col = 1; col < DUNGEON_GEN_WIDTH; col += 2) {
            if (dungeon->blocks[row][col].type != ROCK) {
                continue;
            }

            // carve this section of the maze
            IntList carved_list = init_list();
            dungeon->blocks[row][col].type = HALL;
            list_push(&carved_list, (row * DUNGEON_WIDTH) + col);

            int last_dir = better_rand(3);
            while(carved_list.size > 0) {
                //print_dungeon(dungeon);
                int coord = list_pop(&carved_list);

                // decompose the coordinate
                int col = coord % DUNGEON_WIDTH;
                int row = coord / DUNGEON_WIDTH;


                DungeonBlock *adjacent[4][2] = {{&dungeon->blocks[row - 1][col], &dungeon->blocks[row - 2][col] },
                                                {&dungeon->blocks[row][col + 1], &dungeon->blocks[row][col + 2] },
                                                {&dungeon->blocks[row + 1][col], &dungeon->blocks[row + 2][col] },
                                                {&dungeon->blocks[row][col - 1], &dungeon->blocks[row][col - 2] }};
                
                int lowest_weight = INT_MAX;
                int lowest_weight_index = 0;
                bool can_carve[4] = {false, false, false, false};
                for (int i = 0; i < 4; i++) {
                    DungeonBlock *block_a = adjacent[i][0];
                    DungeonBlock *block_b = adjacent[i][1];
                    can_carve[i] = (block_a->hardness < 4 && block_a->type == ROCK) && (block_b->hardness < 4 && block_b->type == ROCK);

                    if (!can_carve[i]) {
                        continue;
                    }

                    int weight = block_a->hardness + block_b->hardness;
                    if (i == last_dir) {
                        weight -= 2;
                    }

                    if (weight < lowest_weight) {
                        lowest_weight = weight;
                        lowest_weight_index = i;
                    }
                }
                
                if (lowest_weight == INT_MAX) {
                    last_dir = better_rand(3);
                    continue;
                }

                int next_dir;
                if (better_rand(100) > windiness && can_carve[lowest_weight_index]) {
                    next_dir = lowest_weight_index;
                } else {
                    do {
                        next_dir = better_rand(3);
                    } while(!can_carve[next_dir]);
                }

                int coord_a;
                int coord_b;
                switch(next_dir) {
                    default:
                    case 0:
                        coord_a = ((row - 1) * DUNGEON_WIDTH) + col;
                        coord_b = ((row - 2) * DUNGEON_WIDTH) + col;
                        break;
                    case 1:
                        coord_a = (row * DUNGEON_WIDTH) + (col + 1);
                        coord_b = (row * DUNGEON_WIDTH) + (col + 2);
                        break;
                    case 2:
                        coord_a = ((row + 1) * DUNGEON_WIDTH) + col;
                        coord_b = ((row + 2) * DUNGEON_WIDTH) + col;
                        break;
                    case 3:
                        coord_a = (row * DUNGEON_WIDTH) + (col - 1);
                        coord_b = (row * DUNGEON_WIDTH) + (col - 2);
                        break;
                } 

                dungeon->blocks[coord_a / DUNGEON_WIDTH][coord_a % DUNGEON_WIDTH].type = HALL;
                dungeon->blocks[coord_b / DUNGEON_WIDTH][coord_b % DUNGEON_WIDTH].type = HALL;
                
                last_dir = next_dir;
                list_push(&carved_list, coord);
                list_push(&carved_list, coord_b);
            }
            destroy_list(&carved_list);
        }
    }
}
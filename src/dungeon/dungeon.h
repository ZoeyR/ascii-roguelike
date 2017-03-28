#ifndef DUNGEON_H
#define DUNGEON_H

#include <dungeon/entities.h>

#define ROOM_MAX_WIDTH 27
#define ROOM_MAX_HEIGHT 27
#define DUNGEON_WIDTH 160
#define DUNGEON_HEIGHT 105

#define HARDNESS_TIER_0 0
#define HARDNESS_TIER_1 85
#define HARDNESS_TIER_2 171
#define HARDNESS_TIER_3 220
#define HARDNESS_TIER_MAX 255

typedef struct {
    int save;
    int load;
    char path[256];
    int full_size;
    int monsters;
    int room_tries;
    int min_rooms;
    int hardness;
    int windiness;
    int max_maze_size;
    int imperfection;
} Options;

typedef struct {
    enum Type {
        ROCK,
        HALL,
        FLOOR,
        RUBBLE,
        PILLAR,
        UPSTAIRS,
        DOWNSTAIRS,
    } type;
    uint8_t hardness;
    int region;
    bool immutable;
    EIdx entity_id;
} DungeonBlock;

typedef struct {
    char start_row;
    char start_col;
    char width;
    char height;
    DungeonBlock blocks[27][27];
} DungeonRoom;

typedef struct {
    Options params;
    int regions;
    int monster_count;
    EIdx player_id;
    EntityStore *store;
    DungeonBlock blocks[105][160];
} Dungeon;

// create a new random room with the given paramters. Rooms must be no larget than 25x25
DungeonRoom create_room(int width, int height);

Dungeon create_dungeon(Options params);

void rebuild_dungeon(Dungeon *dungeon);

void destroy_dungeon(Dungeon *dungeon);

void merge_regions(Dungeon *dungeon, int extra_hole_chance);

#endif

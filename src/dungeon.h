#ifndef ROOM_H
#define ROOM_H

#define ROOM_MAX_WIDTH 27
#define ROOM_MAX_HEIGHT 27
typedef struct {
    enum {
        ROCK,
        WALL,
        HALL,
        FLOOR,
        RUBBLE,
        PILLAR
    } type;
    char hardness;
} DungeonBlock;

typedef struct {
    char start_row;
    char start_col;
    char width;
    char height;
    DungeonBlock blocks[27][27];
} DungeonRoom;

typedef struct {
    DungeonBlock blocks[105][160];
} Dungeon;

// create a new random room with the given paramters. Rooms must be no larget than 25x25
DungeonRoom create_room(int width, int height);

Dungeon create_dungeon(DungeonRoom *rooms, int len);

#endif

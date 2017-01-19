#include "dungeon.h"
#include <math.h>
#include <stdlib.h>

static DungeonRoom create_rectangle(int width, int height);
//static DungeonRoom create_ellipse(int width, int height);
static void decorate_room(DungeonRoom *room);

DungeonRoom create_room(int width, int height) {
    // Determine the type of room to make, currently ellipse and rectangle
    int choice = rand() % 2;
    DungeonRoom room;
    switch(choice) {
        default:
        case 0:
            room = create_rectangle(width, height);
            break;
        case 1:
            room = create_rectangle(width, height);
            break;
    }

    // decorate the room with odds and ends

    return room;
}

Dungeon create_dungeon(void) {
    Dungeon dungeon;
    // fill dungeon with random noise
    for(int row = 0; row < 105; row++) {
        for(int col = 0; col < 160; col++) {
            char hardness = (rand() % 2) + 1;
            DungeonBlock block = {.type = ROCK, .hardness = hardness };
            dungeon.blocks[row][col] = block;
        }
    }

    // generate enough rooms for the dungeon to be reasonably filled
    int total_rock = 105 * 160;
    int percent_rock = 100;
    int rooms = 0;
    while(rooms < 10 && percent_rock > 80) {
        int width = (rand() % 21) + 7;
        int height = (rand() % 23) + 5;

        DungeonRoom room = create_room(width, height);
        
        // attempt to place room 5 times
        for(int attempt = 0; attempt < 5; attempt++) {
            int col = rand() % (105 - width);
            int row = rand() % (160 - height);

            if (!can_place_room(&room, col, row)) {
                continue;
            } else {
                place_room(&dungeon, &room, col, row);
                total_rock -= width * height;
                percent_rock = (total_rock / (105 * 160)) * 100;
                break;
            }
    }

    // create veins of hard and soft rock

    // path between the rooms
}

static DungeonRoom create_rectangle(int width, int height) {
    // calculate the start row and column, rooms are centered in the array
    int start_row = (ROOM_MAX_HEIGHT/2) - (height / 2);
    int start_col = (ROOM_MAX_WIDTH/2) - (width / 2);

    DungeonRoom room;
    room.start_row = start_row;
    room.start_col = start_col;
    room.width = width;
    room.height = height;
    
    // first fill the room with rock
    for(int row = 0; row < ROOM_MAX_HEIGHT; row++) {
        for(int col = 0; col < ROOM_MAX_WIDTH; col++) {
            char hardness = (rand() % 2) + 1;
            DungeonBlock block = {.type = ROCK, .hardness = hardness};
            room.blocks[row][col] = block;
        }
    }

    // then fill in the room floor + walls
    for(int row = start_row; row < start_row + height; row++) {
        for(int col = start_col; col < start_col + width; col++) {
            char type;
            if (col == start_col || col == (start_col + width - 1) || 
                    row == start_row || row == (start_row + height - 1)) {
                type = WALL;
            } else {
                type = FLOOR;
            }

            room.blocks[row][col].type = type;
        }
    }

    // finally decorate the room
    decorate_room(&room, width, height);

    return room;
}

// currently places pillars in the room, walls and pillars can support a
// ceiling in a 9x9 area. So pillars are places appropriately
static void decorate_room(DungeonRoom *room) {
    char width = room->width;
    char height = room->height;
    int uncovered_width = (width - 10);
    int uncovered_height = (height - 10);
    
    int start_row = (ROOM_MAX_HEIGHT/2) - (height/2);
    int start_col = (ROOM_MAX_WIDTH/2) - (width/2);

    for(int row = 5 + start_row; row < uncovered_height + 5 + start_row; row += 9) {
        for(int col = 5 + start_col; col < uncovered_width + 5 + start_col; col += 9) {
            room->blocks[row][col].type = PILLAR;
            room->blocks[row][2 * start_col + width - col - 1].type = PILLAR;
            room->blocks[2 * start_row + height - row - 1][col].type = PILLAR;
            room->blocks[2 * start_row + height - row - 1]
                [2 * start_col + width - col - 1].type = PILLAR;
        }
    }
}

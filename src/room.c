#include "dungeon.h"
#include <stdlib.h>
#include <math.h>

static DungeonRoom create_rectangle(int width, int height);
static DungeonRoom create_ellipse(int width, int height);
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
            room = create_ellipse(width, height);
            break;
    }

    // decorate the room with odds and ends

    return room;
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
            DungeonBlock block = {.type = ROCK, .hardness = hardness, .region = 0, .immutable = false};
            room.blocks[row][col] = block;
        }
    }

    // then fill in the room floor + walls
    for(int row = start_row; row < start_row + height; row++) {
        for(int col = start_col; col < start_col + width; col++) {
            char type;
            if (col == start_col || col == (start_col + width - 1) || 
                    row == start_row || row == (start_row + height - 1)) {
                type = ROCK;
            } else {
                type = FLOOR;
            }

            room.blocks[row][col].type = type;
        }
    }

    // finally decorate the room
    decorate_room(&room);

    return room;
}

static DungeonRoom create_ellipse(int width, int height) {
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
            DungeonBlock block = {.type = ROCK, .hardness = hardness, .region = 0, .immutable = false};
            room.blocks[row][col] = block;
        }
    }

    double x_radius_2 = pow(width/2, 2.0);
    double y_radius_2 = pow(height/2, 2.0);

    for(int row = 0; row < ROOM_MAX_HEIGHT; row++) {
        for(int col = 0; col < ROOM_MAX_WIDTH; col++) {
            char type;
            double x_2 = pow(col - (ROOM_MAX_WIDTH / 2), 2.0);
            double y_2 = pow(row - (ROOM_MAX_HEIGHT / 2), 2.0);

            if (((x_2/x_radius_2) + (y_2/y_radius_2)) < 1) {
                type = FLOOR;
            } else {
                type = ROCK;
            }

            room.blocks[row][col].type = type;
        }
    }

    // finally decorate the room
    decorate_room(&room);

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
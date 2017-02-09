#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <endian.h>
#include <limits.h>

#include <dungeon/dungeon.h>
#include <io.h>

#define S_HARDNESS_0 "\033[1;7;33;40m%c\033[0m"
#define S_HARDNESS_1 "\033[1;7;33;40m%c\033[0m"
#define S_HARDNESS_2 "\033[1;7;37;40m%c\033[0m"
#define S_HARDNESS_3 "\033[30;47m%c\033[0m"
#define S_HARDNESS_4 "\033[1;7;30;40m%c\033[0m"
#define S_HARDNESS_5 "\033[30;41m%c\033[0m"

#define HARDNESS_0 "\033[1;33;40m%c\033[0m"
#define HARDNESS_1 "\033[1;33;40m%c\033[0m"
#define HARDNESS_2 "\033[1;37;40m%c\033[0m"
#define HARDNESS_3 "\033[37;40m%c\033[0m"
#define HARDNESS_4 "\033[1;30;40m%c\033[0m"
#define HARDNESS_5 "\033[31;40m%c\033[0m"

#define DISTANCE_0 "\033[1;7;35;40m%c\033[0m"
#define DISTANCE_1 "\033[37;44m%c\033[0m"
#define DISTANCE_2 "\033[1;7;34;47m%c\033[0m"
#define DISTANCE_3 "\033[37;42m%c\033[0m"
#define DISTANCE_4 "\033[1;7;32;40m%c\033[0m"
#define DISTANCE_5 "\033[37;43m%c\033[0m"
#define DISTANCE_6 "\033[1;7;33;40m%c\033[0m"
#define DISTANCE_7 "\033[37;41m%c\033[0m"
#define DISTANCE_8 "\033[1;7;31;40m%c\033[0m"
#define DISTANCE_9 "\033[37;45m%c\033[0m"

static void print_block(DungeonBlock block, bool visible);
static void print_hardness(char c, DungeonBlock block);
static void print_s_hardness(char c, DungeonBlock block);
static void print_distance(int distance);

typedef union {
    uint32_t num;
    uint8_t bytes[4];
} FileNum;

void print_dungeon(Dungeon *dungeon) {
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            int right = (col >= DUNGEON_WIDTH - 1) ? 0 : col + 1;
            int left = (col <= 0) ? DUNGEON_WIDTH - 1 : col - 1;
            int top = (row <= 0) ? DUNGEON_HEIGHT - 1 : row - 1;
            int bottom = (row >= DUNGEON_HEIGHT - 1) ? 0 : row + 1;
            bool visible = false;

            visible |= dungeon->blocks[top][left].type != ROCK;
            visible |= dungeon->blocks[top][col].type != ROCK;
            visible |= dungeon->blocks[top][right].type != ROCK;
            visible |= dungeon->blocks[row][left].type != ROCK;
            visible |= dungeon->blocks[row][right].type != ROCK;
            visible |= dungeon->blocks[bottom][left].type != ROCK;
            visible |= dungeon->blocks[bottom][col].type != ROCK;
            visible |= dungeon->blocks[bottom][right].type != ROCK;

            if (dungeon->player_loc[0] == row && dungeon->player_loc[1] == col) {
                printf("\033[1;32;40m@\033[0m");
            } else {
                print_block(dungeon->blocks[row][col], visible);
            }
        }
        printf("\n");
    }
}

void print_room(DungeonRoom *room) {
    for(int row = 0; row < ROOM_MAX_HEIGHT; row++) {
        for(int col = 0; col < ROOM_MAX_WIDTH; col++) {
            int right = (col >= ROOM_MAX_WIDTH - 1) ? 0 : col + 1;
            int left = (col <= 0) ? ROOM_MAX_WIDTH - 1 : col - 1;
            int top = (row <= 0) ? ROOM_MAX_HEIGHT - 1 : row - 1;
            int bottom = (row >= ROOM_MAX_HEIGHT - 1) ? 0 : row + 1;
            bool visible = false;

            visible |= room->blocks[top][left].type != ROCK;
            visible |= room->blocks[top][col].type != ROCK;
            visible |= room->blocks[top][right].type != ROCK;
            visible |= room->blocks[row][left].type != ROCK;
            visible |= room->blocks[row][right].type != ROCK;
            visible |= room->blocks[bottom][left].type != ROCK;
            visible |= room->blocks[bottom][col].type != ROCK;
            visible |= room->blocks[bottom][right].type != ROCK;

            print_block(room->blocks[row][col], visible);
        }
        printf("\n");
    }
}

void print_distance_map(Distances* distances) {
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            int distance = distances->d[row][col];
            print_distance(distance);
        }
        printf("\n");
    }
}

void save_dungeon(Dungeon *dungeon, char* path) {
    FILE *file = fopen(path, "wb");
    fputs("RLG327-S2017", file);

    FileNum version = {.num = 0};
    version.num = htobe32(version.num);
    fputc(version.bytes[0], file);
    fputc(version.bytes[1], file);
    fputc(version.bytes[2], file);
    fputc(version.bytes[3], file);

    fputc(0, file);
    fputc(0, file);
    fputc(0, file);
    fputc(0, file);

    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            DungeonBlock block = dungeon->blocks[row][col];
            if(block.type != ROCK) {
                fputc(0, file);
            } else if (block.immutable) {
                fputc(255, file);
            } else {
                fputc(block.hardness, file);
            }
        }
    }

    int room_store_size = 0;
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            DungeonBlock block = dungeon->blocks[row][col];
            if(block.type != ROCK && block.type != HALL) {
                fputc(col, file);
                fputc(row, file);
                fputc(1, file);
                fputc(1, file);
                room_store_size += 4;
            }
        }
    }

    FileNum magic = {.num = 0x0BADF00D};
    magic.num = htobe32(magic.num);
    fputc(magic.bytes[0], file);
    fputc(magic.bytes[1], file);
    fputc(magic.bytes[2], file);
    fputc(magic.bytes[3], file);

    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            // load row major type and real hardness
            fputc(dungeon->blocks[row][col].hardness, file);
            fputc(dungeon->blocks[row][col].type, file);
        }
    }

    fseek(file, 16, SEEK_SET);
    
    FileNum size = {.num = room_store_size + 16820};
    size.num = htobe32(size.num);
    fputc(size.bytes[0], file);
    fputc(size.bytes[1], file);
    fputc(size.bytes[2], file);
    fputc(size.bytes[3], file);

    fclose(file);
}

Dungeon load_dungeon(char* path) {
    Dungeon dungeon;
    FILE *file = fopen(path, "rb");
    fseek(file, 16, SEEK_SET);

    FileNum size;
    size.bytes[0] = fgetc(file);
    size.bytes[1] = fgetc(file);
    size.bytes[2] = fgetc(file);
    size.bytes[3] = fgetc(file);
    size.num = be32toh(size.num);

    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            uint8_t hardness = fgetc(file);
            dungeon.blocks[row][col].hardness = hardness;
            dungeon.blocks[row][col].region = 0;

            if (hardness == 0) {
                dungeon.blocks[row][col].type = HALL;
                dungeon.blocks[row][col].immutable = false;
            } else if (hardness == 255) {
                dungeon.blocks[row][col].type = ROCK;
                dungeon.blocks[row][col].immutable = true;
            } else {
                dungeon.blocks[row][col].type = ROCK;
            }
        }
    }

    for(uint32_t i = 0; i < size.num - 16820; i += 4) {
        int col = fgetc(file);
        int row = fgetc(file);
        int width = fgetc(file);
        int height = fgetc(file);

        for(int h = 0; h < height; h++) {
            for(int w = 0; w < width; w++) {
                dungeon.blocks[row + h][col + w].type = FLOOR;
            }
        }
    }

    
    // if we have reached end of file return, otherwise we have special information to load
    if(feof(file)) {
        fclose(file);
        return dungeon;
    }

    FileNum magic;
    magic.bytes[0] = fgetc(file);
    magic.bytes[1] = fgetc(file);
    magic.bytes[2] = fgetc(file);
    magic.bytes[3] = fgetc(file);
    magic.num = be32toh(magic.num);

    if (magic.num != 0x0BADF00D) {
        fclose(file);
        return dungeon;
    }

    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            // load row major type and real hardness
            dungeon.blocks[row][col].hardness = fgetc(file);
            dungeon.blocks[row][col].type = fgetc(file);
        }
    }
    
    fclose(file);
    return dungeon;
}

static void print_block(DungeonBlock block, bool visible) {
    char c;
    switch(block.type) {
        default:
        case ROCK:
            if (visible) {
                print_s_hardness(' ', block);
            } else {
                printf(" ");
            }
            return;
        case HALL:
            c = ':';
            break;
        case FLOOR:
            c = '.';
            break;
        case RUBBLE:
            c = 'r';
            break;
        case PILLAR:
            c = 'I';
            break;
    }
    print_hardness(c, block);
}

static void print_s_hardness(char c, DungeonBlock block) {
    if (block.immutable) {
        printf(S_HARDNESS_5, c);
        return;
    }
    switch(block.hardness) {
        case 0:
        case 1:
            printf(S_HARDNESS_0, c);
            break;
        case 2:
            printf(S_HARDNESS_2, c);
            break;
        case 3:
            printf(S_HARDNESS_3, c);
            break;
        default:
        case 4:
            printf(S_HARDNESS_4, c);
            break;
        case 255:
            printf(S_HARDNESS_5, c);
            break;
    }
}

static void print_hardness(char c, DungeonBlock block) {
    if (block.immutable) {
        printf(S_HARDNESS_5, c);
        return;
    }
    switch(block.hardness) {
        case 0:
        case 1:
            printf(HARDNESS_0, c);
            break;
        case 2:
            printf(HARDNESS_2, c);
            break;
        case 3:
            printf(HARDNESS_3, c);
            break;
        default:
        case 4:
            printf(HARDNESS_4, c);
            break;
        case 255:
            printf(HARDNESS_5, c);
            break;
    }
}

static void print_distance(int distance) {
    if (distance < INT_MAX) {
        switch (distance % 10) {
            case 0:
                printf(DISTANCE_0, '0');
                break;
            case 1:
                printf(DISTANCE_1, '1');
                break;
            case 2:
                printf(DISTANCE_2, '2');
                break;
            case 3:
                printf(DISTANCE_3, '3');
                break;
            case 4:
                printf(DISTANCE_4, '4');
                break;
            case 5:
                printf(DISTANCE_5, '5');
                break;
            case 6:
                printf(DISTANCE_6, '6');
                break;
            case 7:
                printf(DISTANCE_7, '7');
                break;
            case 8:
                printf(DISTANCE_8, '8');
                break;
            case 9:
                printf(DISTANCE_9, '9');
                break;
        }
    } else {
        putchar(' ');
    }
}

                

#include <stdio.h>
#include "dungeon.h"
#include "output.h"


#define S_HARDNESS_0 "\033[1;7;33;40m%c\033[0m"
#define S_HARDNESS_1 "\033[1;7;37;40m%c\033[0m"
#define S_HARDNESS_2 "\033[30;47m%c\033[0m"
#define S_HARDNESS_3 "\033[1;7;30;40m%c\033[0m"
#define S_HARDNESS_4 "\033[30;41m%c\033[0m"

#define HARDNESS_0 "\033[1;33;40m%c\033[0m"
#define HARDNESS_1 "\033[1;37;40m%c\033[0m"
#define HARDNESS_2 "\033[37;40m%c\033[0m"
#define HARDNESS_3 "\033[1;30;40m%c\033[0m"
#define HARDNESS_4 "\033[31;40m%c\033[0m"

static void print_block(DungeonBlock block);
static void print_hardness(char c, char hardness);
static void print_s_hardness(char c, char hardness);

void print_room(DungeonRoom *room) {
    for(int row = 0; row < ROOM_MAX_HEIGHT; row++) {
        for(int col = 0; col < ROOM_MAX_WIDTH; col++) {
            print_block(room->blocks[row][col]);
        }
        printf("\n");
    }
}

static void print_block(DungeonBlock block) {
    char c;
    switch(block.type) {
        default:
        case ROCK:
            printf(" ");
            return;
        case WALL:
            c = ' ';
            print_s_hardness(c, block.hardness);
            return;
        case HALL:
            c = ',';
            break;
        case FLOOR:
            c = '+';
            break;
        case RUBBLE:
            c = 'r';
            break;
        case PILLAR:
            c = 'I';
            break;
    }
    print_hardness(c, block.hardness);
}

static void print_s_hardness(char c, char hardness) {
    switch(hardness) {
        default:
        case 0:
            printf(S_HARDNESS_0, c);
            break;
        case 1:
            printf(S_HARDNESS_1, c);
            break;
        case 2:
            printf(S_HARDNESS_2, c);
            break;
        case 3:
            printf(S_HARDNESS_3, c);
            break;
        case 4:
            printf(S_HARDNESS_4, c);
            break;
    }
}

static void print_hardness(char c, char hardness) {
    switch(hardness) {
        default:
        case 0:
            printf(HARDNESS_0, c);
            break;
        case 1:
            printf(HARDNESS_1, c);
            break;
        case 2:
            printf(HARDNESS_2, c);
            break;
        case 3:
            printf(HARDNESS_3, c);
            break;
        case 4:
            printf(HARDNESS_4, c);
            break;
    }
}

                

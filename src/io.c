#include <util/util.h>
#include <dungeon/dungeon.h>
#include <io.h>

#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <endian.h>
#include <limits.h>
#include <ncurses.h>

#define S_HARDNESS_0 "\033[1;7;33;40m%c\033[0m"
#define S_HARDNESS_1 "\033[1;7;37;40m%c\033[0m"
#define S_HARDNESS_2 "\033[30;47m%c\033[0m"
#define S_HARDNESS_3 "\033[1;7;30;40m%c\033[0m"
#define S_HARDNESS_MAX "\033[30;41m%c\033[0m"

#define HARDNESS_0 "\033[1;33;40m%c\033[0m"
#define HARDNESS_1 "\033[1;37;40m%c\033[0m"
#define HARDNESS_2 "\033[37;40m%c\033[0m"
#define HARDNESS_3 "\033[1;30;40m%c\033[0m"
#define HARDNESS_MAX "\033[31;40m%c\033[0m"

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

static void print_block(DungeonBlock block, bool visible, int row, int col);
static void print_entity(Entity *entity, int row, int col);
static void print_hardness(char c, DungeonBlock block, int row, int col);
static void print_s_hardness(char c, DungeonBlock block, int row, int col);
static void print_distance(int distance);

typedef union {
    uint32_t num;
    uint8_t bytes[4];
} FileNum;

static int SCREEN_ROWS = 24;
static int SCREEN_COLS = 80;

static int GAME_SCREEN_ROWS = 21;
static int GAME_SCREEN_COLS = 80;

static WINDOW *main_screen = NULL;
static WINDOW *game_screen = NULL;

static void cleanup(void) {
    delwin(game_screen);
    endwin();
}

void init_screen(bool full_size) {
    atexit(cleanup);
    initscr();
    cbreak();
    raw();
    keypad(stdscr, true);
    noecho();
    start_color();
    curs_set(0);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_BLACK, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_BLACK, COLOR_BLUE);

    if (full_size) {
        SCREEN_ROWS = LINES;
        SCREEN_COLS = COLS;
        GAME_SCREEN_ROWS = SCREEN_ROWS - 3;
        GAME_SCREEN_COLS = SCREEN_COLS;
    }

    int start_row = (LINES - SCREEN_ROWS) / 2;
    int start_col = (COLS - SCREEN_COLS) / 2;


    main_screen = newwin(SCREEN_ROWS, SCREEN_COLS, start_row, start_col);
    game_screen = derwin(main_screen, GAME_SCREEN_ROWS, GAME_SCREEN_COLS, 1, 0);
    notimeout(game_screen, true);
    box(game_screen, 0, 0);
    wbkgd(main_screen, COLOR_PAIR(6));
    wrefresh(main_screen);
}

int get_input(void) {
    return wgetch(game_screen);
}

void print_dungeon(View *view, int center_row, int center_col) {
    wbkgd(main_screen, COLOR_PAIR(6));
    int start_row = center_row - (GAME_SCREEN_ROWS / 2);
    int end_row = start_row + GAME_SCREEN_ROWS;
    int start_col = center_col - (GAME_SCREEN_COLS / 2);
    int end_col = start_col + GAME_SCREEN_COLS;
    for(int row = start_row; row < end_row; row++) {
        for(int col = start_col; col < end_col; col++) {
            // need special logic if we are printing outside view bounds
            if (col < 0 || col >= DUNGEON_WIDTH || row < 0 || row >= DUNGEON_HEIGHT) {
                mvwprintw(game_screen, row - start_row, col - start_col, " ");
                continue;
            }

            int right = (col >= DUNGEON_WIDTH - 1) ? 0 : col + 1;
            int left = (col <= 0) ? DUNGEON_WIDTH - 1 : col - 1;
            int top = (row <= 0) ? DUNGEON_HEIGHT - 1 : row - 1;
            int bottom = (row >= DUNGEON_HEIGHT - 1) ? 0 : row + 1;
            bool visible = false;

            visible |= view->blocks[top][left].type != ROCK;
            visible |= view->blocks[top][col].type != ROCK;
            visible |= view->blocks[top][right].type != ROCK;
            visible |= view->blocks[row][left].type != ROCK;
            visible |= view->blocks[row][right].type != ROCK;
            visible |= view->blocks[bottom][left].type != ROCK;
            visible |= view->blocks[bottom][col].type != ROCK;
            visible |= view->blocks[bottom][right].type != ROCK;

            if (view->blocks[row][col].entity_id != 0) {
                print_entity(unwrap(entity_retrieve(dungeon->store, dungeon->blocks[row][col].entity_id), 1), row - start_row, col - start_col);
            } else {
                print_block(dungeon->blocks[row][col], visible, row - start_row, col - start_col);
            }
        }
    }
    box(game_screen, 0, 0);
    wrefresh(main_screen);
}

void print_distance_map(Distances* distances) {
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            int distance = distances->d[row][col];
            if (distance == 0) {
                printf("\033[1;32;40m@\033[0m");
            } else {
                print_distance(distance);
            }
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

static void print_block(DungeonBlock block, bool visible, int row, int col) {
    char c;
    switch(block.type) {
        default:
        case ROCK:
            if (visible) {
                print_s_hardness(' ', block, row, col);
            } else {
                mvwprintw(game_screen, row, col, " ");
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
        case UPSTAIRS:
            c = '<';
            break;
        case DOWNSTAIRS:
            c = '>';
            break;
    }

    print_hardness(c, block, row, col);
}

static void print_entity(Entity *entity, int row, int col) {
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    if (is_player(entity)) {
        wattron(game_screen, COLOR_PAIR(5));
        mvwprintw(game_screen, row, col, "@");
        return;
    }

    uint32_t print = 0;
    print |= is_smart(entity) & 0x01;
    print |= is_telepathic(entity) & 0x01 << 1;
    print |= is_tunneling(entity) & 0x01 << 2;
    print |= is_erratic(entity) & 0x01 << 3;
    wattron(game_screen, COLOR_PAIR(4));
    mvwprintw(game_screen, row, col, "%x", print);
}

static void print_s_hardness(char c, DungeonBlock block, int row, int col) {
    wattron(game_screen, A_REVERSE);
    if (block.immutable) {
        wattron(game_screen, COLOR_PAIR(4));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(4));
    } else if (block.hardness < HARDNESS_TIER_1) {
        wattron(game_screen, A_BOLD);
        wattron(game_screen, COLOR_PAIR(1));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(1));
        wattroff(game_screen, A_BOLD);
    } else if (block.hardness < HARDNESS_TIER_2) {
        wattron(game_screen, A_BOLD);
        wattron(game_screen, COLOR_PAIR(2));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(2));
        wattroff(game_screen, A_BOLD);
    } else if (block.hardness < HARDNESS_TIER_3) {
        wattron(game_screen, COLOR_PAIR(2));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(2));
    } else if (block.hardness < HARDNESS_TIER_MAX) {
        wattron(game_screen, A_BOLD);
        wattron(game_screen, COLOR_PAIR(3));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(3));
        wattroff(game_screen, A_BOLD);
    } else {
        wattron(game_screen, COLOR_PAIR(4));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(4));
    }
    wattroff(game_screen, A_REVERSE);
}

static void print_hardness(char c, DungeonBlock block, int row, int col) {
    if (block.immutable) {
        wattron(game_screen, COLOR_PAIR(4));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(4));
    } else if (block.hardness < HARDNESS_TIER_1) {
        wattron(game_screen, A_BOLD);
        wattron(game_screen, COLOR_PAIR(1));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(1));
        wattroff(game_screen, A_BOLD);
    } else if (block.hardness < HARDNESS_TIER_2) {
        wattron(game_screen, A_BOLD);
        wattron(game_screen, COLOR_PAIR(2));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(2));
        wattroff(game_screen, A_BOLD);
    } else if (block.hardness < HARDNESS_TIER_3) {
        wattron(game_screen, COLOR_PAIR(2));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(2));
    } else if (block.hardness < HARDNESS_TIER_MAX) {
        wattron(game_screen, A_BOLD);
        wattron(game_screen, COLOR_PAIR(3));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(3));
        wattroff(game_screen, A_BOLD);
    } else {
        wattron(game_screen, COLOR_PAIR(4));
        mvwprintw(game_screen, row, col, "%c", c);
        wattroff(game_screen, COLOR_PAIR(4));
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
        putchar('X');
    }
}               

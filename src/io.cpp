#include <util/util.h>
#include <dungeon/dungeon.h>
#include <io.h>
#include <sstream>
#include <climits>
#include <ncurses.h>
#include <iostream>

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
static void print_object(Object *entity, int row, int col);
static void print_hardness(char c, DungeonBlock block, int row, int col);
static void print_s_hardness(char c, DungeonBlock block, int row, int col);
static void print_distance(int distance);
static int _string_to_color(std::string& color_str);
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

    // OBJECT AND MONSTER COLOR DEFS
    init_pair(10, COLOR_RED, COLOR_BLACK);
    init_pair(11, COLOR_GREEN, COLOR_BLACK);
    init_pair(12, COLOR_BLUE, COLOR_BLACK);
    init_pair(13, COLOR_CYAN, COLOR_BLACK);
    init_pair(14, COLOR_YELLOW, COLOR_BLACK);
    init_pair(15, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(16, COLOR_WHITE, COLOR_BLACK);
    init_pair(17, COLOR_BLACK, COLOR_WHITE);

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

void print_view(GameState *state, int center_row, int center_col) {
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

            visible |= state->view.blocks[top][left].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[top][col].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[top][right].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[row][left].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[row][right].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[bottom][left].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[bottom][col].type != DungeonBlock::ROCK;
            visible |= state->view.blocks[bottom][right].type != DungeonBlock::ROCK;

            if (state->view.blocks[row][col].entity_id != 0) {
                print_entity(state->dungeon.store->get(state->view.blocks[row][col].entity_id).unwrap(), row - start_row, col - start_col);
            } else if (state->view.blocks[row][col].object_id != 0) {
                print_object(state->dungeon.o_store->get(state->view.blocks[row][col].object_id).unwrap(), row - start_row, col - start_col);;
            } else {
                print_block(state->view.blocks[row][col], visible, row - start_row, col - start_col);
            }
        }
    }
    box(game_screen, 0, 0);
    wrefresh(main_screen);
}

void print_dungeon(Dungeon *dungeon, int center_row, int center_col) {
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

            visible |= dungeon->blocks[top][left].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[top][col].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[top][right].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[row][left].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[row][right].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[bottom][left].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[bottom][col].type != DungeonBlock::ROCK;
            visible |= dungeon->blocks[bottom][right].type != DungeonBlock::ROCK;

            if (dungeon->blocks[row][col].entity_id != 0) {
                print_entity(dungeon->store->get(dungeon->blocks[row][col].entity_id).unwrap(), row - start_row, col - start_col);
            } else if (dungeon->blocks[row][col].object_id != 0) {
                print_object(dungeon->o_store->get(dungeon->blocks[row][col].object_id).unwrap(), row - start_row, col - start_col);;
            } else {
                print_block(dungeon->blocks[row][col], visible, row - start_row, col - start_col);
            }
        }
    }
    box(game_screen, 0, 0);
    wrefresh(main_screen);
}

static bool _load_monster(std::istream& is, MonsterDescription& desc) {
    std::string read;
    int parameters_read = 0;

    std::getline(is, read);
    std::cout << "read: " << read << std::endl;
    if (read.compare("BEGIN MONSTER") != 0) {
        return false;
    }

    std::getline(is, read);
    while (read.compare("END") != 0) {
        std::cout << "read: " << read << read.compare("DESC") << std::endl;
        if (read.compare(0, 4, "NAME") == 0) {
            std::cout << "setting name" << std::endl;
            desc.name = read.substr(read.find_first_not_of(' ', 4), std::string::npos);
            parameters_read++;
        } else if (read.compare(0, 4, "SYMB") == 0) {
            desc.symbol = read[5];
            parameters_read++;
        } else if (read.compare(0, 4, "DESC") == 0) {
            std::cout << "description line: " << std::endl;
            std::getline(is, read);
            while(read.compare(".") != 0) {
                if (read.length() > 77) {
                    return false;
                }
                desc.description += read;
                desc.description += "\n";
                std::getline(is, read);
            }
            parameters_read++;
        } else if (read.compare(0, 5, "COLOR") == 0) {
            desc.color = read.substr(read.find_first_not_of(' ', 5), std::string::npos);
            parameters_read++;
        } else if (read.compare(0, 5, "SPEED") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 5), std::string::npos);
            if (!desc.speed.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 4, "ABIL") == 0) {
            std::stringstream ss;
            ss.str(read.substr(read.find_first_not_of(' ', 4), std::string::npos));

            std::string ability;
            while(std::getline(ss, ability, ' ')) {
                std::cout << "ability: " << ability << std::endl;
                if (ability.compare("SMART") == 0) {
                    desc.smart = true;
                } else if (ability.compare("TELE") == 0) {
                    desc.telepathic = true;
                } else if (ability.compare("TUNNEL") == 0) {
                    desc.tunneling = true;
                } else if (ability.compare("ERRATIC") == 0) {
                    desc.erratic = true;
                } else if (ability.compare("PASS") == 0) {
                    desc.pass = true;
                } else if (ability.compare("PICKUP") == 0) {
                    desc.pickup = true;
                } else if (ability.compare("DESTROY") == 0) {
                    desc.destroy = true;
                } else {
                    return false;
                }
            }
            parameters_read++;
        } else if (read.compare(0, 2, "HP") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 2), std::string::npos);
            if (!desc.hp.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 3, "DAM") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 3), std::string::npos);
            if (!desc.damage.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else {
            return false;
        }
        std::getline(is, read);
    }
    std::cout << "done with while, read:" << read << std::endl;

    if (parameters_read != 8) {
        return false;
    }

    return true;
}

std::vector<MonsterDescription> load_desciptions(std::istream& is) {
    std::vector<MonsterDescription> descs;
    std::string desc_header;

    std::getline(is, desc_header);
    if (desc_header.compare("RLG327 MONSTER DESCRIPTION 1") != 0) {
        return descs;
    }

    while (!is.eof()) {
        std::cout << "reading monster" << std::endl;
        std::getline(is, desc_header);
        MonsterDescription desc;
        auto succ = _load_monster(is, desc);
        std::cout << "read monster" << std::endl;
        if (!succ) {
            continue;
        }

        descs.push_back(desc);
    }
    
    return descs;
}

static bool _load_object(std::istream& is, ObjectDescription& desc) {
    std::string read;
    int parameters_read = 0;

    std::getline(is, read);
    std::cout << "read: " << read << std::endl;
    if (read.compare("BEGIN OBJECT") != 0) {
        return false;
    }

    std::getline(is, read);
    while (read.compare("END") != 0) {
        if (read.compare(0, 4, "NAME") == 0) {
            desc.name = read.substr(read.find_first_not_of(' ', 4), std::string::npos);
            parameters_read++;
        } else if (read.compare(0, 4, "DESC") == 0) {
            std::getline(is, read);
            while(read.compare(".") != 0) {
                if (read.length() > 77) {
                    return false;
                }
                desc.description += read;
                desc.description += "\n";
                std::getline(is, read);
            }
            parameters_read++;
        } else if (read.compare(0, 4, "TYPE") == 0) {
            auto type_string = read.substr(read.find_first_not_of(' ', 4), std::string::npos);
            Result<ObjectType, Unit> res = object_type_from_string(type_string);
            if (res.is_err()) {
                return false;
            }
            desc.type = res.unwrap();
            parameters_read++;
        } else if (read.compare(0, 5, "COLOR") == 0) {
            desc.color = read.substr(read.find_first_not_of(' ', 5), std::string::npos);
            parameters_read++;
        } else if (read.compare(0, 3, "HIT") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 3), std::string::npos);
            if (!desc.hit_bonus.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 3, "DAM") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 3), std::string::npos);
            if (!desc.damage_bonus.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 5, "DODGE") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 5), std::string::npos);
            if (!desc.dodge_bonus.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 3, "DEF") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 3), std::string::npos);
            if (!desc.defense_bonus.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 6, "WEIGHT") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 6), std::string::npos);
            if (!desc.weight.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 5, "SPEED") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 5), std::string::npos);
            if (!desc.speed_bonus.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 4, "ATTR") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 4), std::string::npos);
            if (!desc.special.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else if (read.compare(0, 3, "VAL") == 0) {
            auto dice_str = read.substr(read.find_first_not_of(' ', 3), std::string::npos);
            if (!desc.value.parse_str(dice_str)) {
                return false;
            }
            parameters_read++;
        } else {
            return false;
        }
        std::getline(is, read);
    }
    std::cout << "done with while, read:" << parameters_read << std::endl;

    if (parameters_read != 12) {
        return false;
    }

    return true;
}

std::vector<ObjectDescription> load_object_descriptions(std::istream& is) {
    std::vector<ObjectDescription> descs;
    std::string desc_header;

    std::getline(is, desc_header);
    if (desc_header.compare("RLG327 OBJECT DESCRIPTION 1") != 0) {
        return descs;
    }

    while (!is.eof()) {
        std::cout << "reading object" << std::endl;
        //std::getline(is, desc_header);
        ObjectDescription desc;
        auto succ = _load_object(is, desc);
        std::cout << "read object" << std::endl;
        if (!succ) {
            continue;
        }

        descs.push_back(desc);
    }
    std::cout << "read " << descs.size() << " objects" << std::endl;
    return descs;
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
            if(block.type != DungeonBlock::ROCK) {
                fputc(0, file);
            } else if (block.immutable) {
                fputc(255, file);
            } else {
                fputc(block.hardness, file);
            }
        }
    }

    uint32_t room_store_size = 0;
    for(int row = 0; row < DUNGEON_HEIGHT; row++) {
        for(int col = 0; col < DUNGEON_WIDTH; col++) {
            DungeonBlock block = dungeon->blocks[row][col];
            if(block.type != DungeonBlock::ROCK && block.type != DungeonBlock::HALL) {
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
                dungeon.blocks[row][col].type = DungeonBlock::HALL;
                dungeon.blocks[row][col].immutable = false;
            } else if (hardness == 255) {
                dungeon.blocks[row][col].type = DungeonBlock::ROCK;
                dungeon.blocks[row][col].immutable = true;
            } else {
                dungeon.blocks[row][col].type = DungeonBlock::ROCK;
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
                dungeon.blocks[row + h][col + w].type = DungeonBlock::FLOOR;
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
            dungeon.blocks[row][col].type = static_cast<DungeonBlock::Type>(fgetc(file));
        }
    }
    
    fclose(file);
    return dungeon;
}

static void print_block(DungeonBlock block, bool visible, int row, int col) {
    char c;
    switch(block.type) {
        default:
        case DungeonBlock::ROCK:
            if (visible) {
                print_s_hardness(' ', block, row, col);
            } else {
                mvwprintw(game_screen, row, col, " ");
            }
            return;
        case DungeonBlock::HALL:
            c = ':';
            break;
        case DungeonBlock::FLOOR:
            c = '.';
            break;
        case DungeonBlock::RUBBLE:
            c = 'r';
            break;
        case DungeonBlock::PILLAR:
            c = 'I';
            break;
        case DungeonBlock::UPSTAIRS:
            c = '<';
            break;
        case DungeonBlock::DOWNSTAIRS:
            c = '>';
            break;
    }

    print_hardness(c, block, row, col);
}

static void print_entity(Entity *entity, int row, int col) {
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    if (is_player(entity)) {
        wattron(game_screen, COLOR_PAIR(5));
    } else {
        int color = _string_to_color(((Monster *)entity)->color);
        wattron(game_screen, COLOR_PAIR(color));
    }
    mvwprintw(game_screen, row, col, "%c", entity->print);
}

static void print_object(Object *object, int row, int col) {
    int color = _string_to_color(object->color);
    wattron(game_screen, COLOR_PAIR(color));
    mvwprintw(game_screen, row, col, "%c", object->print);
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

static int _string_to_color(std::string& color_str) {
    if (color_str.compare("RED") == 0) {
        return 10;
    } else if (color_str.compare("GREEN") == 0) {
        return 11;
    } else if (color_str.compare("BLUE") == 0) {
        return 12;
    } else if (color_str.compare("CYAN") == 0) {
        return 13;
    } else if (color_str.compare("YELLOW") == 0) {
        return 14;
    } else if (color_str.compare("MAGENTA") == 0) {
        return 15;
    } else if (color_str.compare("WHITE") == 0) {
        return 16;
    } else if (color_str.compare("BLACK") == 0) {
        return 17;
    } else {
        return 3;
    }
}          

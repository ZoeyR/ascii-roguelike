#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include <dungeon/dungeon.h>
#include <util/distance.h>
#include <io.h>

typedef struct {
    int save;
    int load;
    char path[256];
} Options;

Options parse_args(int argc, char *argv[]);
static int _length_no_tunnel(void *context, Coordinate *this, Coordinate *to);
static int _length_tunnel(void *context, Coordinate *this, Coordinate *to);
int main(int argc, char *argv[]) {
    srand(time(NULL));
    int room_tries = 1000;
    int min_rooms = 10;
    int hardness = 50;
    int windiness = 30;
    int max_maze_size = 2000;
    int imperfection = 2000;
    Options options = parse_args(argc, argv);

    Dungeon dungeon;
    if (options.load) {
        dungeon = load_dungeon(options.path);
    } else {
        dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, max_maze_size, imperfection);
    }

    print_dungeon(&dungeon);

    Distances no_tunnel = dijkstra(&dungeon, dungeon.player_loc[0], dungeon.player_loc[1], _length_no_tunnel);
    print_distance_map(&dungeon, &no_tunnel);

    Distances tunnel = dijkstra(&dungeon, dungeon.player_loc[0], dungeon.player_loc[1], _length_tunnel);
    print_distance_map(&dungeon, &tunnel);

    if (options.save) {
        save_dungeon(&dungeon, options.path);
    }
}

static int _length_no_tunnel(void *context, Coordinate *this, Coordinate *to) {
    Dungeon *dungeon = (Dungeon *)context;
    (void)(this);
    DungeonBlock b_to = dungeon->blocks[to->row][to->col];
    if (b_to.type == ROCK || b_to.type == PILLAR) {
        return INT_MAX;
    } else {
        return 1;
    }
}

static int _length_tunnel(void *context, Coordinate *this, Coordinate *to) {
    Dungeon *dungeon = (Dungeon *)context;
    (void)(this);
    DungeonBlock b_to = dungeon->blocks[to->row][to->col];
    if (b_to.type == ROCK || b_to.type == PILLAR) {
        if(b_to.immutable) {
            return INT_MAX;
        }

        uint8_t hardness = b_to.hardness;
        if (hardness < HARDNESS_TIER_1) {
            return 1;
        } else if (hardness < HARDNESS_TIER_2) {
            return 2;
        } else if (hardness < HARDNESS_TIER_3) {
            return 3;
        } else if (hardness < HARDNESS_TIER_MAX) {
            return 4;
        } else {
            return INT_MAX;
        }
    } else {
        return 1;
    }
}

Options parse_args(int argc, char *argv[]) {
    Options options = {.save = false, .load = false};
    strcpy(options.path, getenv("HOME"));
    strcat(options.path, "/.rlg327/");
    mkdir(options.path, 0777);
    strcat(options.path, "dungeon");
    
    struct option long_options[] = { {"save", no_argument, &options.save, true},
                                     {"load", no_argument, &options.load, true},
                                     {"path", required_argument, 0, 'p'}};
    int option_index = 0;

    int c;
    while((c = getopt_long(argc, argv, "slp:", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                options.save = true;
                break;
            case 'l':
                options.load = true;
                break;
            case 'p':
                strcpy(options.path, optarg);
                break;
            default:
                break;
        }
    }
    
    return options;
}

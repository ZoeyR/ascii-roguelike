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
#include <util/util.h>
#include <loop.h>
#include <io.h>
#include <unistd.h>

typedef struct {
    int save;
    int load;
    char path[256];
    int monsters;
} Options;

Options parse_args(int argc, char *argv[]);
int main(int argc, char *argv[]) {
    srand(time(NULL));
    int room_tries = 1000;
    int min_rooms = 10;
    int hardness = 50;
    int windiness = 30;
    int max_maze_size = 2000;
    int imperfection = 2000;
    Options options = parse_args(argc, argv);

    init_screen();
    Dungeon dungeon;
    if (options.load) {
        dungeon = load_dungeon(options.path);
    } else {
        dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, max_maze_size, imperfection, options.monsters);
    }

    GameState state = init_state(&dungeon);

    while (1) {
        Entity *player = unwrap(entity_retrieve(&dungeon.store, dungeon.player_id), 1);
        print_dungeon(&dungeon, player->player.row, player->player.col);
        tick(&dungeon, &state);

        if (!unwrap(entity_retrieve(&dungeon.store, dungeon.player_id), 1)->alive) {
            printf("Player loses :(\n");
            break;
        } 
        if (dungeon.monster_count == 0) {
            printf("Player wins!\n");
            break;
        }
    }

    if (options.save) {
        save_dungeon(&dungeon, options.path);
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
                                     {"path", required_argument, NULL, 'p'},
                                     {"nummon", required_argument, NULL, 'n'}};
    int option_index = 0;

    int c;
    while((c = getopt_long(argc, argv, "slp:n:", long_options, &option_index)) != -1) {
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
            case 'n':
                options.monsters = expect(parse_int(optarg), "nummon argument must be an integer", 1);
                break;
            default:
                break;
        }
    }
    
    return options;
}

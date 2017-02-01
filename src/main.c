#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>

#include <dungeon/dungeon.h>
#include <io.h>

typedef struct {
    int save;
    int load;
    char path[256];
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

    printf("save: %d, load: %d\npath: %s", options.save, options.load, options.path);

    Dungeon dungeon;
    if (options.load) {
        dungeon = load_dungeon(options.path);
    } else {
        dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, max_maze_size, imperfection);
    }

    print_dungeon(&dungeon);
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

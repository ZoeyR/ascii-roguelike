#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>

#include <dungeon/dungeon.h>
#include <output.h>

typedef struct {
    int save;
    int load;
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

    printf("save: %d, load: %d\n", options.save, options.load);
    Dungeon dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, max_maze_size, imperfection);

    print_dungeon(&dungeon);
}

Options parse_args(int argc, char *argv[]) {
    Options options = {.save = false, .load = false};
    struct option long_options[] = { {"save", no_argument, &options.save, true},
                                     {"load", no_argument, &options.load, true}};
    int option_index = 0;

    int c;
    while((c = getopt_long(argc, argv, "sl", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                options.save = true;
                break;
            case 'l':
                options.load = true;
                break;
            default:
                break;
        }
    }
    
    return options;
}

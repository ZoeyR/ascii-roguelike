#include <getopt.h>
#include <cstring>
#include <climits>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>

#include <dungeon/dungeon.h>
#include <util/distance.h>
#include <util/util.h>
#include <loop.h>
#include <io.h>

Options parse_args(int argc, char *argv[]);
int main() {
    /*
    srand(time(NULL));
    
    Options options = parse_args(argc, argv);
    options.room_tries = 1000;
    options.min_rooms = 10;
    options.hardness = 50;
    options.windiness = 30;
    options.max_maze_size = 2000;
    options.imperfection = 2000;
    
    init_screen(options.full_size);
    Dungeon dungeon;
    if (options.load) {
        dungeon = load_dungeon(options.path);
    } else {
        dungeon = create_dungeon(options);
    }

    GameState state = GameState(dungeon);
    while (1) {
        Entity *player = state.dungeon.store->get(state.dungeon.player_id).unwrap();
        print_view(&state, player->row, player->col);
        state.tick();
        if (!state.dungeon.store->get(state.dungeon.player_id).unwrap()->alive) {
            printf("Player loses :(\n");
            break;
        } 
    }

    if (options.save) {
        save_dungeon(&state.dungeon, options.path);
    }
    destroy_state(&state);
    */
    std::string path = getenv("HOME");
    path += "/.rlg327/";
    mkdir(path.c_str(), 0777);
    path += "monster_desc.txt";
    std::ifstream file_stream(path, std::ifstream::in); 
    auto descriptions = load_desciptions(file_stream);
    std::cout << descriptions.size() << " Monsters read" << std::endl;
    for(size_t i = 0; i < descriptions.size(); i++) {
        descriptions[i].print();
        std::cout << std::endl;
    }
    return 0;
}

Options parse_args(int argc, char *argv[]) {
    Options options;
    options.save = false;
    options.load = false;

    strcpy(options.path, getenv("HOME"));
    strcat(options.path, "/.rlg327/");
    mkdir(options.path, 0777);
    strcat(options.path, "dungeon");
    
    struct option long_options[] = { {"save", no_argument, &options.save, true},
                                     {"load", no_argument, &options.load, true},
                                     {"path", required_argument, NULL, 'p'},
                                     {"nummon", required_argument, NULL, 'n'},
                                     {"full", no_argument, &options.full_size, true}};
    int option_index = 0;

    int c;
    while((c = getopt_long(argc, argv, "slfp:n:", long_options, &option_index)) != -1) {
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
                options.monsters = parse_int(optarg).expect("nummon argument must be an integer");
                break;
            case 'f':
                options.full_size = true;
                break;
            default:
                break;
        }
    }
    
    return options;
}

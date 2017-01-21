#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <dungeon/dungeon.h>
#include <output.h>

int main(int argc, char *argv[]) {
    int room_tries;
    int min_rooms;
    int hardness;
    int windiness;
    int max_maze_size;
    int imperfection;

    if (argc < 8) {
        srand(time(NULL));
        room_tries = 1000;
        min_rooms = 10;
        hardness = 50;
        windiness = 30;
        max_maze_size = 2000;
        imperfection = 2000;
    } else {
        srand(atoi(argv[1]));
        room_tries = atoi(argv[2]);
        min_rooms = atoi(argv[3]);
        hardness = atoi(argv[4]);
        windiness = atoi(argv[5]);
        max_maze_size = atoi(argv[6]);
        imperfection = atoi(argv[7]);
    }

    Dungeon dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, max_maze_size, imperfection);

    print_dungeon(&dungeon);
}

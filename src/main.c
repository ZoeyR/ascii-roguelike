#include <stdlib.h>
#include <stdio.h>

#include <dungeon/dungeon.h>
#include <output.h>

int main(int argc, char *argv[]) {
    if (argc < 8) {
        return 1;
    }
    
    srand(atoi(argv[1]));

    int room_tries = atoi(argv[2]);
    int min_rooms = atoi(argv[3]);
    int hardness = atoi(argv[4]);
    int windiness = atoi(argv[5]);
    int max_maze_size = atoi(argv[6]);
    int imperfection = atoi(argv[7]);

    Dungeon dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, max_maze_size, imperfection);

    print_dungeon(&dungeon);
    printf("regions: %d", dungeon.regions);
}

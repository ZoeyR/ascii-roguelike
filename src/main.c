#include "dungeon.h"
#include "output.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    srand(atoi(argv[1]));

    int room_tries = atoi(argv[2]);
    int min_rooms = atoi(argv[3]);
    int hardness = atoi(argv[4]);
    int windiness = atoi(argv[5]);
    int imperfection = atoi(argv[6]);

    Dungeon dungeon = create_dungeon(room_tries, min_rooms, hardness, windiness, imperfection);

    print_dungeon(&dungeon);
    printf("regions: %d", dungeon.regions);
}

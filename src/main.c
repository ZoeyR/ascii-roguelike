#include "dungeon.h"
#include "output.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    srand(atoi(argv[1]));

    int room_tries = atoi(argv[2]);
    int min_rooms = atoi(argv[3]);
    int hardness = atoi(argv[4]);

    Dungeon dungeon = create_dungeon(room_tries, min_rooms, hardness);

    print_dungeon(&dungeon);
    printf("done");
}

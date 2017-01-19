#include "dungeon.h"
#include "output.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    DungeonRoom room = create_room(width, height);

    print_room(&room);
}

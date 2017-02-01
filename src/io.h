#ifndef IO_H
#define IO_H

#include <dungeon/dungeon.h>

void print_room(DungeonRoom *room);

void print_dungeon(Dungeon *dungeon);

void save_dungeon(Dungeon *dungeon, char *path);

Dungeon load_dungeon(char *path);

#endif

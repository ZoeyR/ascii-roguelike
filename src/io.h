#ifndef IO_H
#define IO_H

#include <dungeon/entities.h>
#include <util/distance.h>
#include <dungeon/dungeon.h>

void init_screen(void);

int get_input(void);

void print_dungeon(Dungeon *dungeon, int center_row, int center_col);

void print_distance_map(Distances* distances);

void save_dungeon(Dungeon *dungeon, char *path);

Dungeon load_dungeon(char *path);

#endif

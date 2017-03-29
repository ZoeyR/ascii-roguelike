#ifndef IO_H
#define IO_H

#include <vector>
#include <istream>
#include <dungeon/entities.h>
#include <util/distance.h>
#include <dungeon/dungeon.h>
#include <loop.h>

void init_screen(bool full_size);

int get_input(void);

void print_view(GameState *state, int center_row, int center_col);

void print_dungeon(Dungeon *dungeon, int center_row, int center_col);

void print_distance_map(Distances* distances);

void save_dungeon(Dungeon *dungeon, char *path);

Dungeon load_dungeon(char *path);

std::vector<MonsterDescription> load_desciptions(std::istream& is);
#endif

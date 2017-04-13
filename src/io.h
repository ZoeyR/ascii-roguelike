#ifndef IO_H
#define IO_H

#include <vector>
#include <istream>
#include <dungeon/entities.h>
#include <util/distance.h>
#include <dungeon/dungeon.h>
#include <dungeon/object.h>
#include <loop.h>

void init_screen(bool full_size);

int get_input(void);

void print_view(GameState *state, int center_row, int center_col);

void print_dungeon(const Dungeon *dungeon, int center_row, int center_col);

void print_distance_map(Distances* distances);

void print_pc_inventory(Dungeon *dungeon);

void print_pc_equipment(Dungeon *dungeon);

void print_item_description(Dungeon *dungeon, OIdx index);

int prompt_player(const char* prompt);

void notify(const char* prompt, int row_off);

void save_dungeon(Dungeon *dungeon, char *path);

Dungeon load_dungeon(char *path);

std::vector<MonsterDescription> load_desciptions(std::istream& is);

std::vector<ObjectDescription> load_object_descriptions(std::istream& is);
#endif

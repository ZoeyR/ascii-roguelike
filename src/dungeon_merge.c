#include "dungeon.h"
#include "util.h"
#include <stdbool.h>
#include <stdlib.h>

// comments for merging dungeon
// create array with indicies being region numbers and values being a list of connectors and flag of merged
// for each region in list:
// * if merged, continue
// * shuffle the list
// * while there are connectors in list
//   * pop connector in list
//   * if connector connects to a non-merged region, merge
//   * to merge, turn connector into hall tile, then mark the connected region as merged
//   * small chance to open a connector even if connects to merged region, makes dungeon imperfect.

typedef struct {
    int row;
    int col;
    struct {
        int row;
        int col;
    } regions[2];
} Connector;

typedef struct {
    Connector *data;
    size_t size;
    size_t capacity;
} ConnectorList;

typedef struct {
    int region;
    bool merged;
    ConnectorList list;
} MergeTracker;

static ConnectorList _init_list(void);
static void _destroy_list(ConnectorList *list);
static void _list_push(ConnectorList *list, Connector n);
static Connector _list_pop(ConnectorList *list);
static void _list_shuffle(ConnectorList *list);
static bool _make_connector(Dungeon *dungeon, Connector *connector);
static void _flood_fill(Dungeon *dungeon, int col, int row, int target, int replacement);

void merge_regions(Dungeon *dungeon, int extra_hole_chance) {
    int regions = dungeon->regions;
    MergeTracker *trackers = malloc(sizeof(MergeTracker) * regions);
    
    for(int i = 0; i < regions; i++) {
        ConnectorList list = _init_list();
        trackers[i] = (MergeTracker){.region = i, .merged = false, .list = list};
    }

    // find all connectors in the dungeon
    for (int row = 0; row < DUNGEON_HEIGHT; row++) {
        for (int col = 0; col < DUNGEON_WIDTH; col++) {
            Connector connector = {.row = row, .col = col};
            if (_make_connector(dungeon, &connector)) {
                int col = connector.regions[0].col;
                int row = connector.regions[0].row;
                int region = dungeon->blocks[row][col].region;
                _list_push(&trackers[region - 1].list, connector);
                
                col = connector.regions[1].col;
                row = connector.regions[1].row;
                region = dungeon->blocks[row][col].region;
                _list_push(&trackers[region - 1].list, connector);
            }
        }
    }

    // merge the regions
    for(int i = 0; i < regions; i++) {

        ConnectorList *list = &trackers[i].list;
        _list_shuffle(list);

        while(list->size > 0) {
            Connector connector = _list_pop(list);
            int row_a, col_a, row_b, col_b;
            row_a = connector.regions[0].row;
            col_a = connector.regions[0].col;
            row_b = connector.regions[1].row;
            col_b = connector.regions[1].col;

            if (dungeon->blocks[row_a][col_a].region != dungeon->blocks[row_b][col_b].region) {
                int replacement = dungeon->blocks[row_a][col_a].region;
                int target = dungeon->blocks[row_b][col_b].region;
                dungeon->blocks[connector.row][connector.col].region = target;
                dungeon->blocks[connector.row][connector.col].type = HALL;
                
                // flood fill the region to the same region, doesnt matter which one
                _flood_fill(dungeon, connector.row, connector.col, target, replacement);
                dungeon->regions--;
                continue;
            }

            if (better_rand(extra_hole_chance) == 0) {
                dungeon->blocks[connector.row][connector.col].type = HALL;
            }
        }
    }

    // clean up resources
    for(int i = 0; i < regions; i++) {
        _destroy_list(&trackers[i].list);
    }

    free(trackers);
}

static bool _make_connector(Dungeon *dungeon, Connector *connector) {
    int col = connector->col;
    int row = connector->row;

    if (dungeon->blocks[row][col].type != ROCK) {
        return false;
    }

    relative_array(1, row, col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );

    int adjacent[4][2] = {  {top, col},
                            {row, right},
                            {bottom, col},
                            {row, left}};

    int region_a = -1;
    int region_b = -1;
    for (int i = 0; i < 4; i++) {
        DungeonBlock block = dungeon->blocks[adjacent[i][0]][adjacent[i][1]];
        if (block.type != ROCK) {
            if (region_a == -1) {
                region_a = block.region;
                connector->regions[0].row = adjacent[i][0];
                connector->regions[0].col = adjacent[i][1];
            } else if (block.region != region_a) {
                region_b = block.region;
                connector->regions[1].row = adjacent[i][0];
                connector->regions[1].col = adjacent[i][1];
            }
        }
    }

    return region_b != -1;
}

static void _list_shuffle(ConnectorList *list) {
    for (int i = 0; i < (int)list->size; i++) {
        int swap = i + better_rand(list->size - i - 1);
        Connector tmp = list->data[swap];
        list->data[swap] = list->data[i];
        list->data[i] = tmp;
    }
}

static ConnectorList _init_list(void) {
    size_t capacity = 10;
    size_t size = 0;
    Connector *data = malloc(sizeof(Connector) * capacity);

    return (ConnectorList){.capacity = capacity, .size = size, .data = data};
}

static void _destroy_list(ConnectorList *list) {
    list->size = 0;
    list->capacity = 0;
    free(list->data);
}

static void _list_push(ConnectorList *list, Connector n) {
    list->data[list->size] = n;
    list->size++;

    if (list->size >= list->capacity) {
        // reallocate list
        Connector *new_data = realloc(list->data, sizeof(Connector) * (list->capacity * 2));
        if (!new_data) {
            // error! revert the push
            list->size--;
            return;
        }
        list->data = new_data;
        list->capacity = list->capacity * 2;
    }
}

static Connector _list_pop(ConnectorList *list) {
    list->size--;
    return list->data[list->size];
}

static void _flood_fill(Dungeon *dungeon, int row, int col, int target, int replacement) {
    if (target == replacement) {
        return;
    }

    if (dungeon->blocks[row][col].region != target) {
        return;
    }

    dungeon->blocks[row][col].region = replacement;

    relative_array(1, row, col, DUNGEON_HEIGHT, DUNGEON_WIDTH, );

    _flood_fill(dungeon, top, col, target, replacement);
    _flood_fill(dungeon, row, right, target, replacement);
    _flood_fill(dungeon, bottom, col, target, replacement);
    _flood_fill(dungeon, row, left, target, replacement);
}
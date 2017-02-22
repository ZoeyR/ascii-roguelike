#ifndef ENTITIES_H
#define ENTITIES_H

#include <stdbool.h>
#include <collections/list.h>

typedef size_t EIdx;

typedef struct {
    bool smart :1;
    bool telepathic :1;
    bool tunneling :1;
    bool erratic :1;
    int pc_last_seen[2];
    int row;
    int col;
} MonsterData;

typedef struct {
    int row;
    int col;
} PlayerData;

typedef struct {
    union {
        MonsterData monster;
        PlayerData player;
    };
    enum {
        MONSTER,
        PLAYER,
    } type;
    bool alive;
    int speed;
    EIdx index;
} Entity;

typedef struct {
    List list;
} EntityStore;

define_result(EntityStoreResult, Entity *, Unit);
EntityStore init_entity_store();
EIdx add_entity(EntityStore *store, Entity entity);
EIdx spawn_monster(EntityStore *store, int row, int col);
EntityStoreResult entity_retrieve(EntityStore *store, EIdx index);
#endif
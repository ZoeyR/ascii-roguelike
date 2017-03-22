#ifndef ENTITIES_H
#define ENTITIES_H

#ifdef __cplusplus
#include <vector>
#include <memory>
extern "C" {
#endif
#include <util/util.h>
#include <collections/list.h>
#ifdef __cplusplus
}
#endif
typedef size_t EIdx;

#ifdef __cplusplus


class Entity {
    virtual void no_op();
    public:
        bool alive;
        int row;
        int col;
        int speed;
        EIdx index;
        Entity(int row, int col);
};

class Player: public Entity {
    public:
        Player(int row, int col);
};

class Monster: public Entity {
    public:
        bool smart :1;
        bool telepathic :1;
        bool tunneling :1;
        bool erratic :1;
        int pc_last_seen[2];
        Monster(int row, int col);
};

class EntityStore {
    template <typename E>
    EIdx add_entity(E entity);
    public:
        std::vector<std::unique_ptr<Entity>> list;
        EntityStore();
        ~EntityStore();
        EIdx spawn_player(int row, int col);
        EIdx spawn_monster(int row, int col);
};

#else
typedef void Entity;
typedef void EntityStore;
#endif

#ifdef __cplusplus
extern "C" {
#endif

define_result(EntityStoreResult, Entity *, Unit);
EntityStore* init_entity_store();
void destroy_entity_store(EntityStore *store);
EIdx spawn_monster(EntityStore *store, int row, int col);
EIdx spawn_player(EntityStore *store, int row, int col);
EntityStoreResult entity_retrieve(EntityStore *store, EIdx index);
size_t store_size(EntityStore *store);

bool is_player(Entity *entity);
bool is_smart(Entity *entity);
bool is_telepathic(Entity *entity);
bool is_tunneling(Entity *entity);
bool is_erratic(Entity *entity);
int entity_row(Entity *entity);
int entity_col(Entity *entity);
void set_entity_row(Entity *entity, int row);
void set_entity_col(Entity *entity, int col);
bool entity_alive(Entity *entity);
int entity_speed(Entity *entity);
EIdx entity_index(Entity *entity);
int player_last_seen_row(Entity *entity);
int player_last_seen_col(Entity *entity);
void kill_entity(Entity *entity);
#ifdef __cplusplus
}
#endif

#endif
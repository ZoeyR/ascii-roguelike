#ifndef ENTITIES_H
#define ENTITIES_H

#include <vector>
#include <memory>
#include <util/util.h>
typedef size_t EIdx;

class Entity {
    virtual void no_op();
    public:
        bool alive;
        int row;
        int col;
        int speed;
        char print;
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
    std::vector<std::unique_ptr<Entity>> list;
    public:
        EIdx spawn_player(int row, int col);
        EIdx spawn_monster(int row, int col);
        size_t size();
        Result<Entity *, Unit> get(EIdx index);
};

bool is_player(Entity *entity);

#endif
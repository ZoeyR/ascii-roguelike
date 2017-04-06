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
        std::string name;
        std::string color;
        int hp;
        Dice damage;
        bool smart :1;
        bool telepathic :1;
        bool tunneling :1;
        bool erratic :1;
        int pc_last_seen[2];
        Monster(int row, int col);
};

class EntityStore {
    std::vector<std::unique_ptr<Entity>> list;
    public:
        template <typename E>
        EIdx add_entity(E entity);
        EIdx spawn_player(int row, int col);
        EIdx spawn_monster(int row, int col);
        size_t size();
        Result<Entity *, Unit> get(EIdx index);
};

class MonsterDescription {
    public:
        std::string name;
        std::string description;
        char symbol;
        std::string color;
        Dice speed;
        bool smart :1;
        bool telepathic :1;
        bool tunneling :1;
        bool erratic :1;
        bool pass :1;
        bool pickup :1;
        bool destroy :1;
        Dice hp;
        Dice damage;
        MonsterDescription();
        Monster generate(int row, int col);
        void print();
};

bool is_player(Entity *entity);

#endif
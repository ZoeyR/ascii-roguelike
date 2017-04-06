#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <util/util.h>
#include <vector>
#include <memory>

typedef size_t OIdx;

enum class ObjectType {
    WEAPON,
    OFFHAND,
    RANGED,
    ARMOR,
    HELMET,
    CLOAK,
    GLOVES,
    BOOTS,
    RING,
    AMULET,
    LIGHT,
    SCROLL,
    BOOK,
    FLASK,
    GOLD,
    AMMUNITION,
    FOOD,
    WAND,
    CONTAINER
};

Result<ObjectType, Unit> object_type_from_string(std::string& str);

class Object {
    public:
        OIdx index;
        char print;
        std::string name;
        std::string description;
        ObjectType type;
        std::string color;
        int hit_bonus;
        Dice damage_bonus;
        int defense_bonus;
        int weight;
        int speed_bonus;
        int special;
        int value;
};

class ObjectDescription {
    public:
        std::string name;
        std::string description;
        ObjectType type;
        std::string color;
        Dice hit_bonus;
        Dice damage_bonus;
        Dice dodge_bonus;
        Dice defense_bonus;
        Dice weight;
        Dice speed_bonus;
        Dice special;
        Dice value;
        Object generate();
};



class ObjectStore {
    std::vector<std::unique_ptr<Object>> list;
    public:
        OIdx add_object(Object object);
        size_t size();
        Result<Object *, Unit> get(OIdx index);
};

#endif
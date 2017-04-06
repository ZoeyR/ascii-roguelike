#include <dungeon/object.h>

Result<ObjectType, Unit> object_type_from_string(std::string& str) {
    ObjectType type;
    if (str.compare("WEAPON") == 0) {
        type = ObjectType::WEAPON;
    } else if (str.compare("OFFHAND") == 0) {
        type = ObjectType::OFFHAND;
    } else if (str.compare("RANGED") == 0) {
        type = ObjectType::RANGED;
    } else if (str.compare("ARMOR") == 0) {
        type = ObjectType::ARMOR;
    } else if (str.compare("HELMET") == 0) {
        type = ObjectType::HELMET;
    } else if (str.compare("CLOAK") == 0) {
        type = ObjectType::CLOAK;
    } else if (str.compare("GLOVES") == 0) {
        type = ObjectType::GLOVES;
    } else if (str.compare("BOOTS") == 0) {
        type = ObjectType::BOOTS;
    } else if (str.compare("RING") == 0) {
        type = ObjectType::RING;
    } else if (str.compare("AMULET") == 0) {
        type = ObjectType::AMULET;
    } else if (str.compare("LIGHT") == 0) {
        type = ObjectType::LIGHT;
    } else if (str.compare("SCROLL") == 0) {
        type = ObjectType::SCROLL;
    } else if (str.compare("BOOK") == 0) {
        type = ObjectType::BOOK;
    } else if (str.compare("FLASK") == 0) {
        type = ObjectType::FLASK;
    } else if (str.compare("GOLD") == 0) {
        type = ObjectType::GOLD;
    } else if (str.compare("AMMUNITION") == 0) {
        type = ObjectType::AMMUNITION;
    } else if (str.compare("FOOD") == 0) {
        type = ObjectType::FOOD;
    } else if (str.compare("WAND") == 0) {
        type = ObjectType::WAND;
    } else if (str.compare("CONTAINER") == 0) {
        type = ObjectType::CONTAINER;
    } else {
        return Result<ObjectType, Unit>(unit());
    }

    return type;
}

Object ObjectDescription::generate() {
    Object object;
    object.name = this->name;
    object.description = this->description;
    object.type = this->type;
    object.color = this->color;
    object.hit_bonus = this->hit_bonus.roll();
    object.damage_bonus = this->damage_bonus;
    object.defense_bonus = this->defense_bonus.roll();
    object.weight = this->weight.roll();
    object.speed_bonus = this->speed_bonus.roll();
    object.special = this->special.roll();
    object.value = this->value.roll();
    
    switch (this->type) {
        default:
        case ObjectType::WEAPON:
            object.print = '|';
            break;
        case ObjectType::OFFHAND:
            object.print = ')';
            break;
        case ObjectType::RANGED:
            object.print = '}';
            break;
        case ObjectType::ARMOR:
            object.print = '[';
            break;
        case ObjectType::HELMET:
            object.print = ']';
            break;
        case ObjectType::CLOAK:
            object.print = '(';
            break;
        case ObjectType::GLOVES:
            object.print = '{';
            break;
        case ObjectType::BOOTS:
            object.print = '\\';
            break;
        case ObjectType::RING:
            object.print = '=';
            break;
        case ObjectType::AMULET:
            object.print = '\"';
            break;
        case ObjectType::LIGHT:
            object.print = '_';
            break;
        case ObjectType::SCROLL:
            object.print = '~';
            break;
        case ObjectType::BOOK:
            object.print = '?';
            break;
        case ObjectType::FLASK:
            object.print = '!';
            break;
        case ObjectType::GOLD:
            object.print = '$';
            break;
        case ObjectType::AMMUNITION:
            object.print = '/';
            break;
        case ObjectType::FOOD:
            object.print = ',';
            break;
        case ObjectType::WAND:
            object.print = '-';
            break;
        case ObjectType::CONTAINER:
            object.print = '%';
            break;
    }
    return object;
}

OIdx ObjectStore::add_object(Object object) {
    object.index = list.size() + 1;
    list.push_back(std::make_unique<Object>(object));
    return object.index;
}

size_t ObjectStore::size() {
    return list.size();
}

Result<Object *, Unit> ObjectStore::get(OIdx index) {
    if (index > list.size()) {
        return Result<Object *, Unit>(unit());
    }

    return list[index - 1].get();
}
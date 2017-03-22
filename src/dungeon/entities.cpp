#include <iostream>
#include <vector>
#include <memory>
#include <dungeon/entities.h>
extern "C" {
#include <util/util.h>
#include <collections/list.h>
}

Entity::Entity(int row, int col) {
    this->row = row;
    this->col = col;
    this->alive = true;
}

Monster::Monster(int row, int col): Entity::Entity(row, col) {
    speed = better_rand(15) + 5;
    pc_last_seen[0] = row;
    pc_last_seen[1] = col;  
    smart = better_rand(1);
    telepathic = better_rand(1);
    tunneling = better_rand(1);
    erratic = better_rand(1);
}

void Entity::no_op() {}

Player::Player(int row, int col): Entity::Entity(row, col) {
    speed = 10;
}

EntityStore::EntityStore() {
    list = std::vector<std::unique_ptr<Entity>>();
}

EntityStore::~EntityStore() {
}

template <typename E>
EIdx EntityStore::add_entity(E entity) {
    entity.index = list.size() + 1;
    list.push_back(std::make_unique<E>(entity));
    return entity.index;
}

EIdx EntityStore::spawn_player(int row, int col) {
    Player player(row, col);
    EIdx j = add_entity(player);
    return j;
}

EIdx EntityStore::spawn_monster(int row, int col) {
    Monster monster(row, col);
    return add_entity(monster);
}

extern "C" {

    EntityStore* init_entity_store() {
        return new EntityStore();
    }

    void destroy_entity_store(EntityStore *store) {
        delete store;
    }

    EIdx spawn_monster(EntityStore *store, int row, int col) {
        return store->spawn_monster(row, col);
    }

    EIdx spawn_player(EntityStore *store, int row, int col) {
        return store->spawn_player(row, col);
    }

    EntityStoreResult entity_retrieve(EntityStore *store, EIdx index) {
        Entity * entity = store->list[index - 1].get();
        return (EntityStoreResult)ok(entity);
    }

    size_t store_size(EntityStore *store) {
        return store->list.size();
    }

    bool is_player(Entity *entity) {
      return dynamic_cast<Player *>(entity);
    }

    bool is_smart(Entity *entity) {
        if (Monster * monster = dynamic_cast<Monster *>(entity)) {
            return monster->smart;
        } else {
            return false;
        }
    }

    bool is_telepathic(Entity *entity) {
        if (Monster * monster = dynamic_cast<Monster *>(entity)) {
            return monster->telepathic;
        } else {
          return false;
        }
    }

    bool is_tunneling(Entity *entity) {
        if (Monster * monster = dynamic_cast<Monster *>(entity)) {
            return monster->tunneling;
        } else {
          return false;
        }
    }

    bool is_erratic(Entity *entity) {
        if (Monster * monster = dynamic_cast<Monster *>(entity)) {
            return monster->erratic;
        } else {
          return false;
        }
    }

    int entity_row(Entity *entity) {
        return entity->row;
    }

    int entity_col(Entity *entity) {
        return entity->col;
    }

    void set_entity_row(Entity *entity, int row) {
        entity->row = row;
    }

    void set_entity_col(Entity *entity, int col) {
        entity->col = col;
    }

    bool entity_alive(Entity *entity) {
        return entity->alive;
    }

    int entity_speed(Entity *entity) {
        return entity->speed;
    }

    EIdx entity_index(Entity *entity) {
        return entity->index;
    }

    int player_last_seen_row(Entity *entity) {
        if (Monster * monster = dynamic_cast<Monster *>(entity)) {
            return monster->pc_last_seen[0];
        } else {
          return entity->row;
        }
    }

    int player_last_seen_col(Entity *entity) {
        if (Monster * monster = dynamic_cast<Monster *>(entity)) {
            return monster->pc_last_seen[1];
        } else {
          return entity->col;
        }
    }

    void kill_entity(Entity *entity) {
        entity->alive = false;
    }
}
#include <dungeon/entities.h>
#include <util/util.h>

EntityStore init_entity_store() {
    List list = init_list(sizeof(Entity));

    return (EntityStore){.list = list};
}

void destroy_entity_store(EntityStore *store) {
    destroy_list(&store->list);
}

EIdx spawn_monster(EntityStore *store, int row, int col) {
    Entity monster;
    monster.type = MONSTER;
    monster.speed = better_rand(15) + 5;
    monster.alive = true;
    monster.monster.row = row;
    monster.monster.col = col;
    monster.monster.pc_last_seen[0] = row;
    monster.monster.pc_last_seen[1] = col;

    monster.monster.smart = better_rand(1);
    monster.monster.telepathic = better_rand(1);
    monster.monster.tunneling = better_rand(1);
    monster.monster.erratic = better_rand(1);

    return add_entity(store, monster);
}

EIdx add_entity(EntityStore *store, Entity entity) {
    entity.index = store->list.size + 1;
    list_push(&store->list, &entity);
    return entity.index;
}

EntityStoreResult entity_retrieve(EntityStore *store, EIdx index) {
    Entity * entity = (Entity *)try_result_map(list_at(&store->list, index - 1), EntityStoreResult, unit());
    return (EntityStoreResult)ok(entity);
}
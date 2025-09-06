#ifndef PLAYER_LIST_HPP
#define PLAYER_LIST_HPP

#include "../interfaces/engine.hpp"

class Entity;
class Player;

class EntityList {
public:
    Entity *entity_from_index(unsigned int index) {
        void **vtable = *reinterpret_cast<void ***>(this);

        Entity * (*entity_from_index_fn)(void *, unsigned int) = (Entity* (*)(void *, unsigned int)) vtable[3];

        return entity_from_index_fn(this, index);
    }

    Player *player_from_index(unsigned int index) {
        return reinterpret_cast<Player *>(entity_from_index(index));
    }

    Entity *entity_from_handle(int handle) {
        return entity_from_index(handle & (1 << 16) - 1);
        // Convert handle to index https://github.com/ValveSoftware/source-sdk-2013/blob/39f6dde8fbc238727c020d13b05ecadd31bda4c0/src/public/const.h#L83
    }


    Player *get_localplayer(void) {
        return player_from_index(engine->get_localplayer_index());
    }

    int get_max_entities(void) {
        void **vtable = *reinterpret_cast<void ***>(this);

        int (*get_max_entities_fn)(void *) = (int (*)(void *)) vtable[8];

        return get_max_entities_fn(this);
    }
};

static inline EntityList *entity_list;

#endif

#ifndef PLAYER_LIST_HPP
#define PLAYER_LIST_HPP

#include "../interfaces/engine.hpp"

class Entity;
class Player;

class EntityList {
public:
  Entity* entity_from_index(unsigned int index) {
    void** vtable = *(void ***)this;
     
    Entity* (*entity_from_index_fn)(void*, unsigned int) = (Entity* (*)(void*, unsigned int))vtable[3];
    
    return entity_from_index_fn(this, index);
  }

  Player* player_from_index(unsigned int index) {
    return (Player*)this->entity_from_index(index);
  }

  Entity* entity_from_handle(int handle) {
    return this->entity_from_index((handle & (( 1 << 16) - 1))); // Convert handle to index https://github.com/ValveSoftware/source-sdk-2013/blob/39f6dde8fbc238727c020d13b05ecadd31bda4c0/src/public/const.h#L83
  }
  
  Player* get_localplayer(void) {
    return this->player_from_index(engine->get_localplayer_index());
  }
  
  Entity* get_entity_from_id(int user_id) {
    return this->entity_from_index(engine->get_player_index_from_id(user_id));
  }

  Player* get_player_from_id(int user_id) {
    return this->player_from_index(engine->get_player_index_from_id(user_id));
  }

  
  int get_max_entities(void) {
    void** vtable = *(void ***)this;
    
    int (*get_max_entities_fn)(void*) = (int (*)(void*))vtable[8];

    return get_max_entities_fn(this);
  }
};

static inline EntityList* entity_list;

#endif

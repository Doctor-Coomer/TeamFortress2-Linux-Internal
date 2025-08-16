#ifndef ENGINE_HPP
#define ENGINE_HPP

class Player;
struct player_info;

#include "../vec.hpp"

class Engine {
public:
  int get_localplayer_index(void) {
    void** vtable = *(void ***)this;
    int (*get_localplayer_index_fn)(void*) = (int (*)(void*))vtable[12];
    
    return get_localplayer_index_fn(this);
  }
  
  Vec2 get_screen_size(void) {
    void** vtable = *(void ***)this;

    void (*get_screen_size_fn)(void*, int*, int*) = (void (*)(void*, int*, int*))vtable[5];
    
    int width;
    int height;
    
    get_screen_size_fn(this, &width, &height);

    return Vec2{width, height};
  }

  bool is_in_game(void) {
    void** vtable = *(void ***)this;
    bool (*is_in_game_fn)(void*) = (bool (*)(void*))vtable[26];

    return is_in_game_fn(this);
  }

  const char* get_level_name(void) {
    void** vtable = *(void ***)this;
    const char* (*get_level_name_fn)(void*) = (const char* (*)(void*))vtable[51];
    return get_level_name_fn(this);
  }


  bool get_player_info(int entity_index, player_info* pinfo) {
    void** vtable = *(void ***)this;
    bool (*get_player_info_fn)(void*, int, player_info*) = (bool (*)(void*, int, player_info*))vtable[8];

    return get_player_info_fn(this, entity_index, pinfo);
  }

};

static inline Engine* engine;


#endif

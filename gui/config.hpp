#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>

#include "../vec.hpp"

struct button {
  int button;
  bool waiting = false;
};

struct Aim {
  bool master = true;

  bool auto_shoot = true;
  
  bool silent = true;
  
  struct button key = {.button = -SDL_BUTTON_X1};
  bool use_key = true;
  
  float fov = 45;
  bool draw_fov = false;

  bool auto_scope = false;
  
  bool ignore_friends = true;
};

struct Esp {
  bool master = true;

  struct Player {
    RGBA_float enemy_color = {.r = 1, .g = 0.501960784, .b = 0, .a = 1};
    RGBA_float team_color = {.r = 1, .g = 1, .b = 1, .a = 1};
    RGBA_float friend_color = {.r = 0, .g = 0.862745098, .b = 0.31372549, .a = 1};
    
    bool box = true;
    bool health_bar = true;    
    bool name = true;

    int hb_pos = 0;
    const char* hb_pos_items[2] = { "Left", "Right" };
    
    struct Flags {
      bool target_indicator = true;
      bool friend_indicator = true;
      int pos = 0;
      const char* pos_items[2] = { "Left", "Right" };
    } flags;
    
    bool friends = true;
    bool team = false;
  } player;

  struct Pickup {
    bool box = false;    
    bool name = true;
  } pickup;

  struct Buildings {
    bool box = true;
    bool health_bar = true;    
    bool name = true;

    bool team = false;
  } buildings;
};

struct Visuals {

  struct Removals {
    bool scope = false;
    bool zoom = false;
  } removals;
  
  struct Thirdperson {
    struct button key = {.button = SDL_SCANCODE_LALT};
    bool enabled = false;
    float z = 150.0f;
    float y = 20.0f;
    float x = 0; 
  } thirdperson;
  
  bool override_fov = false;
  float custom_fov = 90;
};

struct Misc {
  bool bhop = true;
  bool bypasspure = true;
  bool no_push = false;
};

struct Debug {
  int font_height = 14;
  int font_weight = 400;
  bool debug_render_all_entities = false;
};

struct Config {
  Aim aimbot;
  Esp esp;
  Visuals visuals;
  Misc misc;
  Debug debug;
};

inline static Config config;


static bool is_button_down(struct button button) {
  if (button.button >= 0) {
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
  
    if (keys[button.button] == 1)
      return true;

    return false;
  } else {
    Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);

    if (mouse_state & SDL_BUTTON(-button.button))
      return true;

    return false;
  }  

  return false;
}

#endif

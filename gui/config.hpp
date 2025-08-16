#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>

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

  bool ignore_friends = true;
};

struct Esp {
  bool master = true;

  struct Player {
    bool box = true;
    bool health_bar = true;    
    bool name = true;
    
    struct Flags {
      bool target_indicator = true;
      bool friend_indicator = true;
    } flags;
    
    bool friends = true;
  } player;

  struct Pickup {
    bool box = false;    
    bool name = true;
  } pickup;
};

struct Visuals {

  struct Removals {
    bool scope = false;
    bool zoom = false;
  } removals;
  
  struct Thirdperson {
    struct button key = {.button = SDL_SCANCODE_LALT};
    bool enabled = false;
    float distance = 150.0f;
  } thirdperson;
  
  bool override_fov = false;
  float custom_fov = 90;
};

struct Misc {
  bool bhop = true;
  bool bypasspure = true;
  bool no_push = false;
};

struct Nav {
  bool master = false;
  bool engine_enabled = false;
  bool draw_overlay = false;
  bool visualizer_3d = false;
};

struct Config {
  Aim aimbot;
  Esp esp;
  Visuals visuals;
  Misc misc;
  Nav nav;
};

inline static Config config;


static bool is_button_down(struct button button) {
  if (button.button >= 0) {
  
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
  
    if (keys[button.button] == 1) {
      return true;
    }

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

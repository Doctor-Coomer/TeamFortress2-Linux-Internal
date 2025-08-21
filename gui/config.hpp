#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>

inline static const char* TF2_CLASS_DISPLAY_NAMES[10] = {
  "None", "Scout", "Sniper", "Soldier", "Demoman", "Medic", "Heavy", "Pyro", "Spy", "Engineer"
};
inline static const char* TF2_CLASS_CMD_TOKENS[10] = {
  "", "scout", "sniper", "soldier", "demoman", "medic", "heavyweapons", "pyro", "spy", "engineer"
};

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

  bool autoscope = true;
  float autoscope_distance = 1850.0f;

  struct Hitscan {
    bool enabled = true;
    enum Modifiers {
      ScopedOnly      = 1 << 0,
      WaitForHeadshot = 1 << 1,
    };
    int modifiers = 0;
    // 0 = Chest, 1 = Head
    int hitbox = 0;
  } hitscan;

  struct Projectile {
    bool enabled = true;
    // 0 = Chest, 1 = Head
    int hitbox = 0;
  } projectile;

  struct Melee {
    bool enabled = true;
    bool predict = true;
    float predict_time = 0.15f;
    bool auto_backstab = true;
    float melee_reach = 72.0f;
    float backstab_cone_deg = 70.0f;
  } melee;
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
  bool autojoin = false;
  int autojoin_class = 0;
};

struct Nav {
  bool master = true;
  bool engine_enabled = true;
  bool visualize_navmesh = false;
  bool visualize_path = false;
  bool navbot = true;
  bool look_at_path = false;
  bool look_at_path_smoothed = false;
  bool scheduler_enabled = true;
  bool snipe_enemies = true;
  struct SnipeRangePerClass {
    float scout    = 900.0f;
    float sniper   = 900.0f;
    float soldier  = 900.0f;
    float demoman  = 900.0f;
    float medic    = 900.0f;
    float heavy    = 900.0f;
    float pyro     = 900.0f;
    float spy      = 900.0f;
    float engineer = 900.0f;
  } snipe_range;
  int   snipe_repath_ticks = 12;
  float snipe_replan_move_threshold = 96.0f;
  bool chase_target = true;
  bool chase_only_melee = true;
  float chase_distance_max = 1500.0f;
  int chase_repath_ticks = 10;
  float chase_replan_move_threshold = 96.0f;
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

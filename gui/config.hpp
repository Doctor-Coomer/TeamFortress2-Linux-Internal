#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>

#include "../classes/player.hpp"

#include "../vec.hpp"

struct button {
  int button;
  bool waiting = false;
};

struct Aim {
  bool master = true;

  bool auto_shoot = true;

  enum class TargetType {
    FOV,
    DISTANCE,
    LEAST_HEALTH,
    MOST_HEALTH
  } target_type = TargetType::FOV;
  
  bool silent = true;
  
  struct button key = {.button = -SDL_BUTTON_X1};
  bool use_key = true;
  
  float fov = 45;
  bool draw_fov = false;
  
  bool auto_scope = false;
  bool auto_unscope = false;
  bool scoped_only = false;
  
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
    
    struct Flags {
      bool target_indicator = true;
      bool friend_indicator = true;
      bool scoped_indicator = false;
    } flags;

    bool enemy = true;
    bool team = false;
    bool friends = true;
  } player;

  struct Pickup {
    bool box = false;    
    bool name = true;
  } pickup;

  struct Intelligence {
    bool box = true;    
    bool name = true;
  } intelligence;
  
  struct Buildings {
    bool box = true;
    bool health_bar = true;    
    bool name = true;

    bool team = false;
  } buildings;
};

struct Chams {
  bool master = true;

  struct Player {
    bool enemy = true;

    enum class MaterialType {
      NONE,
      FLAT,
      SHADED,
      FRESNEL
    };

    struct ChamFlags {      
      bool ignore_z = true;
      bool wireframe = false;
    };
    
    RGBA_float enemy_color = {.r = .8, .g = 0.701960784, .b = .1, .a = 1};
    MaterialType enemy_material_type = MaterialType::FLAT;
    RGBA_float enemy_color_z = {.r = 1, .g = 0.2, .b = .2, .a = 1};
    MaterialType enemy_material_z_type = MaterialType::FLAT;
    ChamFlags enemy_flags;
    RGBA_float enemy_overlay_color = {.r = .8, .g = 0.701960784, .b = .1, .a = 1};
    MaterialType enemy_overlay_material_type = MaterialType::NONE;
    RGBA_float enemy_overlay_color_z = {.r = 1, .g = 0.2, .b = .2, .a = 1};
    MaterialType enemy_overlay_material_z_type = MaterialType::NONE;
    ChamFlags enemy_overlay_flags;

    bool team = false;    
    RGBA_float team_color = {.r = 0, .g = 1, .b = 0, .a = 1};
    MaterialType team_material_type = MaterialType::SHADED;
    RGBA_float team_color_z = {.r = 0, .g = 0.25, .b = 1, .a = 1};    
    MaterialType team_material_z_type = MaterialType::SHADED;
    ChamFlags team_flags;
    
    bool friends = true;
    RGBA_float friend_color = {.r = 0, .g = 0.632745098, .b = 0.31372549, .a = 1};
    MaterialType friend_material_type = MaterialType::FLAT;
    RGBA_float friend_color_z = {.r = 0, .g = 1, .b = 0.20272549, .a = 1};
    MaterialType friend_material_z_type = MaterialType::FLAT;
    ChamFlags friends_flags;

    bool local = false;
    RGBA_float local_color = {.r = 0, .g = 0.8, .b = 0.35, .a = 1};
    MaterialType local_material_type = MaterialType::SHADED;
    ChamFlags local_flags;
  } player;
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

  bool override_viewmodel_fov = false;
  float custom_viewmodel_fov = 70;
};

struct Misc {

  struct Movement {
    bool bhop = true;
    bool no_push = false;
  } movement;


  struct Exploits {
    bool bypasspure = true;
    bool no_engine_sleep = false;

    bool tickbase = false;
    bool tickbase_indicator = true;
    struct button recharge_button = {.button = SDL_SCANCODE_LSHIFT};
  } exploits;
  
  struct Automation {
    bool auto_class_select = false;
    enum tf_class class_selected = tf_class::SNIPER;
  } automation;

  
  struct Menu {
    bool enabled = true;
    std::string text = "I Use Arch BTW!!!";
  } menu;
};

struct Navbot {
  bool master = false;

  bool walk = false;
  
  bool look_at_path = false;
  float look_smoothness = 50;

  bool do_objective = true;
};

struct Debug {
  int font_height = 14;
  int font_weight = 400;
  bool debug_render_all_entities = false;
  bool debug_draw_navbot_path = false;
  bool debug_draw_navmesh = false;
  bool debug_draw_navbot_goal = false;
  bool debug_draw_navbot_current_area = false;
  bool debug_draw_navbot_next_area = false;
  bool show_active_flag_ids_of_players = false; // What a name. Wow.
  bool disable_friend_checks = true;
  
  // PiShock
  bool pishock_master = false;
  bool pishock_on_kill = true;
  bool pishock_on_death = false;
  std::string username = "";
  std::string sharecode = "";
  std::string apikey = "";
  int intensity = 1;
  int duration = 1;
  enum PiShockOperation {
    SHOCK,
    VIBRATE,
    BEEP
  } operation;
};

struct Config {
  Aim aimbot;
  Esp esp;
  Chams chams;
  Visuals visuals;
  Misc misc;
  Navbot navbot;
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

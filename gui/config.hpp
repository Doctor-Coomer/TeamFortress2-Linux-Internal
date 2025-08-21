#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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
  
  float fov = 180;
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
    int hitbox = 1;
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
    bool scope = true;
    bool zoom = true;
  } removals;
  
  struct Thirdperson {
    struct button key = {.button = SDL_SCANCODE_LALT};
    bool enabled = true;
    float distance = 150.0f;
  } thirdperson;
  
  bool override_fov = false;
  float custom_fov = 90;
};

struct Misc {
  bool bhop = true;
  bool bypasspure = true;
  bool no_push = true;
  bool autojoin = true;
  int autojoin_class = 2;
};

struct Nav {
  bool master = true;
  bool engine_enabled = true;
  bool visualize_navmesh = false;
  bool visualize_path = false;
  bool navbot = true;
  bool look_at_path = true;
  bool look_at_path_smoothed = true;
  bool scheduler_enabled = true;
  bool snipe_enemies = true;
  struct SnipeRangePerClass {
    float scout    = 400.0f;
    float sniper   = 1600.0f;
    float soldier  = 900.0f;
    float demoman  = 900.0f;
    float medic    = 900.0f;
    float heavy    = 600.0f;
    float pyro     = 200.0f;
    float spy      = 100.0f;
    float engineer = 400.0f;
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
inline static bool menu_focused = false;


//fileconfigsystem (chatgped i have no idea how this shit works :3)
namespace cfgio {
  inline static const char* kDir = "/opt/tuxbot";
  inline static const char* kExt = ".cfg";
  inline static std::string g_last_err;

  inline static void set_error_from_errno(const char* prefix) {
    std::ostringstream oss;
    oss << prefix << ": " << std::strerror(errno);
    g_last_err = oss.str();
  }

  inline static const char* GetLastError() {
    return g_last_err.empty() ? nullptr : g_last_err.c_str();
  }

  inline static bool EnsureDir() {
    struct stat st{};
    if (stat(kDir, &st) == 0) {
      if (S_ISDIR(st.st_mode)) return true;
      g_last_err = std::string("Path exists but is not a directory: ") + kDir;
      return false;
    }
    if (mkdir(kDir, 0755) == 0) return true;
    set_error_from_errno("mkdir");
    return false;
  }

  inline static std::string WithExt(const std::string& name) {
    if (name.size() >= 4 && name.substr(name.size() - 4) == kExt) return name;
    return name + kExt;
  }

  inline static std::string Join(const std::string& a, const std::string& b) {
    if (!a.empty() && a.back() == '/') return a + b;
    return a + "/" + b;
  }

  inline static void write_kv(std::ofstream& os, const char* key, bool v) {
    os << key << '=' << (v ? 1 : 0) << '\n';
  }
  inline static void write_kv(std::ofstream& os, const char* key, int v) {
    os << key << '=' << v << '\n';
  }
  inline static void write_kv(std::ofstream& os, const char* key, float v) {
    os << key << '=' << v << '\n';
  }

  template<typename T>
  inline bool assign_from_string(const std::string& s, T&) { return false; }
  template<>
  inline bool assign_from_string<bool>(const std::string& v, bool& out) {
    if (v == "1" || v == "true" || v == "True") { out = true; return true; }
    if (v == "0" || v == "false" || v == "False") { out = false; return true; }
    return false;
  }
  template<>
  inline bool assign_from_string<int>(const std::string& v, int& out) {
    try { out = std::stoi(v); return true; } catch (...) { return false; }
  }
  template<>
  inline bool assign_from_string<float>(const std::string& v, float& out) {
    try { out = std::stof(v); return true; } catch (...) { return false; }
  }

  #define CFG_FIELDS \
    X("aim.master", config.aimbot.master) \
    X("aim.auto_shoot", config.aimbot.auto_shoot) \
    X("aim.silent", config.aimbot.silent) \
    X("aim.key", config.aimbot.key.button) \
    X("aim.use_key", config.aimbot.use_key) \
    X("aim.fov", config.aimbot.fov) \
    X("aim.draw_fov", config.aimbot.draw_fov) \
    X("aim.ignore_friends", config.aimbot.ignore_friends) \
    X("aim.autoscope", config.aimbot.autoscope) \
    X("aim.autoscope_distance", config.aimbot.autoscope_distance) \
    X("aim.hitscan.enabled", config.aimbot.hitscan.enabled) \
    X("aim.hitscan.modifiers", config.aimbot.hitscan.modifiers) \
    X("aim.hitscan.hitbox", config.aimbot.hitscan.hitbox) \
    X("aim.projectile.enabled", config.aimbot.projectile.enabled) \
    X("aim.projectile.hitbox", config.aimbot.projectile.hitbox) \
    X("aim.melee.enabled", config.aimbot.melee.enabled) \
    X("aim.melee.predict", config.aimbot.melee.predict) \
    X("aim.melee.predict_time", config.aimbot.melee.predict_time) \
    X("aim.melee.auto_backstab", config.aimbot.melee.auto_backstab) \
    X("aim.melee.melee_reach", config.aimbot.melee.melee_reach) \
    X("aim.melee.backstab_cone_deg", config.aimbot.melee.backstab_cone_deg) \
    /* ESP */ \
    X("esp.master", config.esp.master) \
    X("esp.player.box", config.esp.player.box) \
    X("esp.player.health_bar", config.esp.player.health_bar) \
    X("esp.player.name", config.esp.player.name) \
    X("esp.player.flags.target_indicator", config.esp.player.flags.target_indicator) \
    X("esp.player.flags.friend_indicator", config.esp.player.flags.friend_indicator) \
    X("esp.player.friends", config.esp.player.friends) \
    X("esp.pickup.box", config.esp.pickup.box) \
    X("esp.pickup.name", config.esp.pickup.name) \
    /* Visuals */ \
    X("visuals.removals.scope", config.visuals.removals.scope) \
    X("visuals.removals.zoom", config.visuals.removals.zoom) \
    X("visuals.thirdperson.key", config.visuals.thirdperson.key.button) \
    X("visuals.thirdperson.enabled", config.visuals.thirdperson.enabled) \
    X("visuals.thirdperson.distance", config.visuals.thirdperson.distance) \
    X("visuals.override_fov", config.visuals.override_fov) \
    X("visuals.custom_fov", config.visuals.custom_fov) \
    /* Misc */ \
    X("misc.bhop", config.misc.bhop) \
    X("misc.bypasspure", config.misc.bypasspure) \
    X("misc.no_push", config.misc.no_push) \
    X("misc.autojoin", config.misc.autojoin) \
    X("misc.autojoin_class", config.misc.autojoin_class) \
    /* NavEngine */ \
    X("nav.master", config.nav.master) \
    X("nav.engine_enabled", config.nav.engine_enabled) \
    X("nav.visualize_navmesh", config.nav.visualize_navmesh) \
    X("nav.visualize_path", config.nav.visualize_path) \
    X("nav.navbot", config.nav.navbot) \
    X("nav.look_at_path", config.nav.look_at_path) \
    X("nav.look_at_path_smoothed", config.nav.look_at_path_smoothed) \
    X("nav.scheduler_enabled", config.nav.scheduler_enabled) \
    X("nav.snipe_enemies", config.nav.snipe_enemies) \
    X("nav.snipe_range.scout", config.nav.snipe_range.scout) \
    X("nav.snipe_range.sniper", config.nav.snipe_range.sniper) \
    X("nav.snipe_range.soldier", config.nav.snipe_range.soldier) \
    X("nav.snipe_range.demoman", config.nav.snipe_range.demoman) \
    X("nav.snipe_range.medic", config.nav.snipe_range.medic) \
    X("nav.snipe_range.heavy", config.nav.snipe_range.heavy) \
    X("nav.snipe_range.pyro", config.nav.snipe_range.pyro) \
    X("nav.snipe_range.spy", config.nav.snipe_range.spy) \
    X("nav.snipe_range.engineer", config.nav.snipe_range.engineer) \
    X("nav.snipe_repath_ticks", config.nav.snipe_repath_ticks) \
    X("nav.snipe_replan_move_threshold", config.nav.snipe_replan_move_threshold) \
    X("nav.chase_target", config.nav.chase_target) \
    X("nav.chase_only_melee", config.nav.chase_only_melee) \
    X("nav.chase_distance_max", config.nav.chase_distance_max) \
    X("nav.chase_repath_ticks", config.nav.chase_repath_ticks) \
    X("nav.chase_replan_move_threshold", config.nav.chase_replan_move_threshold)

  inline static bool Save(const std::string& name) {
    g_last_err.clear();
    if (!EnsureDir()) return false;
    const std::string path = Join(kDir, WithExt(name));
    std::ofstream os(path, std::ios::trunc);
    if (!os.is_open()) {
      g_last_err = std::string("failed to open for write: ") + path;
      return false;
    }
    os << "# Tuxbot" << '\n';
    #define X(KEY, FIELD) write_kv(os, KEY, FIELD);
    CFG_FIELDS;
    #undef X

    os.flush();
    return true;
  }

  inline static bool Load(const std::string& name) {
    g_last_err.clear();
    const std::string path = Join(kDir, WithExt(name));
    std::ifstream is(path);
    if (!is.is_open()) {
      g_last_err = std::string("failed to open for read: ") + path;
      return false;
    }
    std::string line;
    while (std::getline(is, line)) {
      if (line.empty() || line[0] == '#') continue;
      const size_t eq = line.find('=');
      if (eq == std::string::npos) continue;
      std::string key = line.substr(0, eq);
      std::string val = line.substr(eq + 1);

      bool matched = false;
      #define X(KEY, FIELD) \
        if (!matched && key == KEY) { matched = assign_from_string(val, FIELD); }
      CFG_FIELDS;
      #undef X
    }
    return true;
  }

  inline static std::vector<std::string> List() {
    std::vector<std::string> out;
    DIR* dir = opendir(kDir);
    if (!dir) {
      return out;
    }
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
      const char* n = ent->d_name;
      if (n[0] == '.') continue;
      std::string s(n);
      if (s.size() >= 4 && s.substr(s.size() - 4) == kExt) {
        out.emplace_back(s.substr(0, s.size() - 4));
      }
    }
    closedir(dir);
    return out;
  }
} // namespace cfgio


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

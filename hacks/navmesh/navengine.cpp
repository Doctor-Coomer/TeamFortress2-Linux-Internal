// melody 17/aug/2025

#include "navengine.hpp"

#include <unistd.h>
#include <limits.h>
#include <cmath>
#include <string>
#include <vector>
#include <cstdint>

#include "navparser.hpp"
#include "../../gui/imgui/dearimgui.hpp"
#include "../../interfaces/debug_overlay.hpp"
#include "../../interfaces/engine.hpp"
#include "../../interfaces/engine_trace.hpp"
#include "../../gui/config.hpp"
#include "pathfinder.hpp"
#include "reachability.hpp"
#include "navbot/nbcore.hpp"
#include "../../classes/player.hpp"
#include "../../interfaces/client.hpp"
#include "../../print.hpp"
#include "../../vec.hpp"
#include "../aimbot/aimbot.hpp"

namespace nav {

static bool g_draw_enabled = false;
static std::string g_last_error;
static std::string g_last_map;

static std::vector<uint32_t> g_vis_path_ids;
static size_t g_vis_next_index = 0;
static uint32_t g_vis_goal = 0;
static Vec3 g_last_look_angles = {};

static std::string Dirname(const std::string &path) {
  size_t pos = path.find_last_of('/');
  if (pos == std::string::npos) return std::string(".");
  return path.substr(0, pos);
}

void SetDrawEnabled(bool enabled) { g_draw_enabled = enabled; }
bool IsDrawEnabled() { return g_draw_enabled; }

const char *GetLastError() { return g_last_error.empty() ? nullptr : g_last_error.c_str(); }

std::string ExtractMapNameFromPath(const char* level_path) {
  if (!level_path) return {};
  std::string s(level_path);
  size_t pos = s.find_last_of('/');
  if (pos != std::string::npos) s = s.substr(pos + 1);
  pos = s.find_last_of('\\');
  if (pos != std::string::npos) s = s.substr(pos + 1);
  if (s.rfind("maps/", 0) == 0) s = s.substr(5);
  if (s.size() > 4 && s.compare(s.size() - 4, 4, ".bsp") == 0) s.resize(s.size() - 4);
  return s;
}

bool EnsureLoadedForCurrentLevel() {
  if (!engine || !engine->is_in_game()) return false;
  const char* level = engine->get_level_name();
  std::string map = ExtractMapNameFromPath(level);
  if (map.empty()) return false;
  if (!IsLoaded() || map != g_last_map) {
    return LoadForMapName(map);
  }
  return true;
}

bool LoadForMapName(const std::string &map_name) {
  g_last_error.clear();
  if (map_name.empty()) {
    g_last_error = "Empty map name";
    return false;
  }
  
  std::string path = std::string("./tf/maps/") + map_name + ".nav";
  if (!Load(path)) {
    const char *err = GetError();
    g_last_error = err ? err : "Unknown error";
    return false;
  }
  g_last_map = map_name;
  return true;
}

void Draw() {
  if (!g_draw_enabled) return;
  (void)EnsureLoadedForCurrentLevel();

  ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.35f);
  if (ImGui::Begin("nav overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
    if (IsLoaded()) {
      const Info &info = GetInfo();
      ImGui::Text("navmesh loaded");
      if (!g_last_map.empty()) ImGui::Text("Map: %s", g_last_map.c_str());
      ImGui::Text("Path: %s", info.path.c_str());
      ImGui::Text("Size: %zu", info.size);
      ImGui::Text("Magic: %s", info.magic_ascii);
      ImGui::Text("Version: %d", info.version);
      ImGui::Separator();
      ImGui::Text("Places: %zu", GetPlaceCount());
      ImGui::Text("Areas: %zu", GetAreaCount());
      // ImGui::Text("Ladders: %zu", GetLadderCount()); no ladders in tf2 xd
    } else {
      ImGui::Text("No navmesh loaded");
      const char *err = GetLastError();
      if (!err) err = GetError();
      if (err) ImGui::TextColored(ImVec4(1,0.4f,0.2f,1), "Error: %s", err);
    }
  }
  ImGui::End();

}

void Visualizer_ClearPath() {
  g_vis_path_ids.clear();
  g_vis_next_index = 0;
  g_vis_goal = 0;
}

void Visualizer_SetPath(const std::vector<uint32_t>& area_ids,
                        size_t next_index,
                        uint32_t goal_area_id) {
  g_vis_path_ids = area_ids;
  g_vis_next_index = next_index <= g_vis_path_ids.size() ? next_index : g_vis_path_ids.size();
  g_vis_goal = goal_area_id;
}

void Visualizer_GetPath(const std::vector<uint32_t>** out_area_ids,
                        size_t* out_next_index,
                        uint32_t* out_goal_area_id) {
  if (out_area_ids) *out_area_ids = &g_vis_path_ids;
  if (out_next_index) *out_next_index = g_vis_next_index;
  if (out_goal_area_id) *out_goal_area_id = g_vis_goal;
}

CreateMoveResult OnCreateMove(Player* localplayer, user_cmd* user_cmd) {
  CreateMoveResult result{};
  if (!user_cmd || !user_cmd->command_number || !localplayer) {
    return result;
  }

  if (!(config.nav.master && config.nav.engine_enabled) || !engine || !engine->is_in_game() || !(localplayer->get_lifestate() == 1)) {
    Visualizer_ClearPath();
    nav::navbot::Reset();
    g_last_look_angles = {};
    return result;
  }

  static std::string s_last_map_for_reset;
  const char* lvl_path = engine ? engine->get_level_name() : nullptr;
  std::string cur_map = ExtractMapNameFromPath(lvl_path);
  if (!cur_map.empty() && cur_map != s_last_map_for_reset) {
    Visualizer_ClearPath();
    nav::path::Reset();
    nav::navbot::Reset();
    s_last_map_for_reset = cur_map;
  }

  if (EnsureLoadedForCurrentLevel() && IsLoaded()) {
    static int tick_throttle = 0;
    if ((tick_throttle++ & 7) == 0) { // ~1/8 ticks
      Vec3 pos = localplayer->get_origin();
      const nav::Area* area = nav::FindBestAreaAtPosition(pos.x, pos.y, pos.z);
      static uint32_t last_area_id = 0;
      if (area) {
        if (area->id != last_area_id) {
          print("nav: player at (%.1f, %.1f, %.1f) in area id=%u place=%u\n", pos.x, pos.y, pos.z, area->id, area->place_id);
          last_area_id = area->id;
        }
      } else if (last_area_id != 0) {
        print("nav: player not in any area\n");
        last_area_id = 0;
      }
    }

    if (config.nav.roam) {
      auto clampf = [](float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); };
      auto ang_norm = [](float a) {
        a = std::fmod(a, 360.0f);
        if (a > 180.0f) a -= 360.0f;
        if (a <= -180.0f) a += 360.0f;
        return a;
      };

      Vec3 orig_view = user_cmd->view_angles;
      float orig_forward = user_cmd->forwardmove;
      float orig_side = user_cmd->sidemove;

      Vec3 me = localplayer->get_origin();
      nav::navbot::RoamOutput ro{};
      if (nav::navbot::Tick(me.x, me.y, me.z, user_cmd->command_number, &ro) && ro.have_waypoint) {
        float dx = ro.waypoint[0] - me.x;
        float dy = ro.waypoint[1] - me.y;
        float dist2 = dx*dx + dy*dy;

        float desired_yaw = std::atan2(dy, dx) * (180.0f / (float)M_PI);
        float cur_yaw = user_cmd->view_angles.y;
        float dyaw = ang_norm(desired_yaw - cur_yaw);
        float rad = dyaw * (float)M_PI / 180.0f;

        float dist = std::sqrt(dist2);
        float step = dist > 120.0f ? 120.0f : (dist > 60.0f ? 60.0f : 40.0f);
        float dirx = std::cos(std::atan2(dy, dx));
        float diry = std::sin(std::atan2(dy, dx));
        Vec3 ahead = Vec3{ me.x + dirx * step, me.y + diry * step, me.z };
        const float slope_lift = nav::reach::kZSlop; // modest lift to ignore tiny floor bumps
        bool forward_clear = true;
        
        float speed = forward_clear ? 420.0f : 220.0f;
        float fwd = std::cos(rad) * speed;
        float side = -std::sin(rad) * speed;
        
        if (!forward_clear) {
          fwd *= 0.35f;
        }

        user_cmd->forwardmove = clampf(fwd, -450.0f, 450.0f);
        user_cmd->sidemove = clampf(side, -450.0f, 450.0f);

        if (ro.perform_crouch_jump) {
          user_cmd->buttons |= IN_DUCK;
          if (localplayer->get_ground_entity()) {
            user_cmd->buttons |= IN_JUMP;
          }
        }

        result.move_set = true;
        result.orig_view = orig_view;
        result.orig_forward = orig_forward;
        result.orig_side = orig_side;

        bool aimbot_active_now = (config.aimbot.master && target_player != nullptr && ((!config.aimbot.use_key) || is_button_down(config.aimbot.key)));
        if (config.nav.look_at_path && !aimbot_active_now) {
          Vec3 eye = localplayer->get_shoot_pos();
          float dy2 = ro.waypoint[1] - eye.y;
          float dx2 = ro.waypoint[0] - eye.x;
          float desired_yaw2 = std::atan2(dy2, dx2) * (180.0f / (float)M_PI);

          if (config.nav.look_at_path_smoothed) {
            float current_yaw = (g_last_look_angles.y == 0.0f && g_last_look_angles.x == 0.0f && g_last_look_angles.z == 0.0f)
                                  ? user_cmd->view_angles.y
                                  : g_last_look_angles.y;
            float delta_yaw = std::remainder(desired_yaw2 - current_yaw, 360.0f);
            const float aim_speed = 25.0f;
            float next_yaw = current_yaw + (delta_yaw / aim_speed);
            next_yaw = ang_norm(next_yaw);
            user_cmd->view_angles.y = next_yaw;
          } else {
            user_cmd->view_angles.y = ang_norm(desired_yaw2);
          }
          user_cmd->view_angles.x = 0.0f;
          g_last_look_angles = user_cmd->view_angles;
          result.look_applied = true;
        }
      }
    }
  }

  return result;
}

} // namespace

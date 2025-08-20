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
#include "../../hooks/weapon_groups.hpp"

namespace nav {

static bool g_draw_enabled = false;
static std::string g_last_error;
static std::string g_last_map;

static std::vector<uint32_t> g_vis_path_ids;
static size_t g_vis_next_index = 0;
static uint32_t g_vis_goal = 0;
static Vec3 g_last_look_angles = {};
static nav::navbot::TaskKind g_last_task = nav::navbot::TaskKind::Roam;
// we gonna replace that
static int g_last_weapon_slot_selected = 0;
static int g_weapon_switch_cooldown = 0;

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

static const char* TaskKindToString(nav::navbot::TaskKind k) {
  switch (k) {
    case nav::navbot::TaskKind::Roam: return "Roam";
    case nav::navbot::TaskKind::Snipe: return "Snipe";
    case nav::navbot::TaskKind::ChaseMelee: return "ChaseMelee";
    case nav::navbot::TaskKind::GetHealth: return "GetHealth";
    case nav::navbot::TaskKind::GetAmmo: return "GetAmmo";
    case nav::navbot::TaskKind::Retreat: return "Retreat";
    default: return "Unknown";
  }
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
      ImGui::Separator();
      ImGui::Text("Task: %s", TaskKindToString(g_last_task));
      if (g_last_weapon_slot_selected != 0) {
        ImGui::Text("Slot: %d", g_last_weapon_slot_selected);
      }
      // ImGui::Text("Ladders: %zu", GetLadderCount()); no ladders in tf2 xd
    } else {
      ImGui::Text("No navmesh loaded");
      const char *err = GetLastError();
      if (!err) err = GetError();
      if (err) ImGui::TextColored(ImVec4(1,0.4f,0.2f,1), "Error: %s", err);
    }
  }
  ImGui::End();

  if (config.nav.visualize_path && overlay) {
    const std::vector<uint32_t>* ids = nullptr;
    size_t next_idx = 0;
    uint32_t goal_id = 0;
    Visualizer_GetPath(&ids, &next_idx, &goal_id);
    if (ids && !ids->empty()) {
      auto* dl = ImGui::GetBackgroundDrawList();
      auto get_center = [](uint32_t area_id, Vec3* out) -> bool {
        const nav::Area* a = nav::path::GetAreaById(area_id);
        if (!a) return false;
        float c[3]; nav::path::GetAreaCenter(a, c);
        out->x = c[0]; out->y = c[1]; out->z = c[2];
        return true;
      };
      Vec3 wp3d1{}, wp2d1{}, wp3d2{}, wp2d2{};
      for (size_t i = 0; i + 1 < ids->size(); ++i) {
        if (!get_center((*ids)[i], &wp3d1) || !get_center((*ids)[i+1], &wp3d2)) continue;
        if (!overlay->world_to_screen(&wp3d1, &wp2d1) || !overlay->world_to_screen(&wp3d2, &wp2d2)) continue;
        ImU32 col = (i < next_idx) ? IM_COL32(160,160,160,180) : IM_COL32(40,220,90,200);
        dl->AddLine(ImVec2(wp2d1.x, wp2d1.y), ImVec2(wp2d2.x, wp2d2.y), col, 2.0f);
      }
      if (next_idx < ids->size()) {
        if (get_center((*ids)[next_idx], &wp3d1) && overlay->world_to_screen(&wp3d1, &wp2d1)) {
          dl->AddCircleFilled(ImVec2(wp2d1.x, wp2d1.y), 4.0f, IM_COL32(255, 210, 20, 220), 8);
        }
      }
      if (goal_id) {
        if (get_center(goal_id, &wp3d1) && overlay->world_to_screen(&wp3d1, &wp2d1)) {
          dl->AddCircle(ImVec2(wp2d1.x, wp2d1.y), 6.0f, IM_COL32(255, 60, 60, 220), 12, 1.8f);
        }
      }
    }
  }
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

    if (config.nav.navbot) {
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

      nav::navbot::BotContext ctx{};
      ctx.me[0] = me.x; ctx.me[1] = me.y; ctx.me[2] = me.z;
      ctx.cmd_number = user_cmd->command_number;
      ctx.scheduler_enabled = config.nav.scheduler_enabled;
      ctx.snipe_enabled = config.nav.snipe_enemies;
      {
        float sr = config.nav.snipe_range.scout; // sensible default
        switch (localplayer->get_tf_class()) {
          case CLASS_SCOUT:       sr = config.nav.snipe_range.scout;    break;
          case CLASS_SNIPER:      sr = config.nav.snipe_range.sniper;   break;
          case CLASS_SOLDIER:     sr = config.nav.snipe_range.soldier;  break;
          case CLASS_DEMOMAN:     sr = config.nav.snipe_range.demoman;  break;
          case CLASS_MEDIC:       sr = config.nav.snipe_range.medic;    break;
          case CLASS_HEAVYWEAPONS:sr = config.nav.snipe_range.heavy;    break;
          case CLASS_PYRO:        sr = config.nav.snipe_range.pyro;     break;
          case CLASS_SPY:         sr = config.nav.snipe_range.spy;      break;
          case CLASS_ENGINEER:    sr = config.nav.snipe_range.engineer; break;
          default: break;
        }
        ctx.snipe_preferred_range = sr;
      }
      ctx.snipe_repath_ticks = config.nav.snipe_repath_ticks;
      ctx.snipe_replan_move_threshold = config.nav.snipe_replan_move_threshold;
      ctx.chase_enabled = config.nav.chase_target;
      ctx.chase_only_when_melee = config.nav.chase_only_melee;
      ctx.chase_distance_max = config.nav.chase_distance_max;
      ctx.chase_repath_ticks = config.nav.chase_repath_ticks;
      ctx.chase_replan_move_threshold = config.nav.chase_replan_move_threshold;

      {
        Weapon* w = localplayer->get_weapon();
        bool is_melee_w = false;
        if (w) {
          int type_id = w->get_type_id();
          int def_id = w->get_def_id();
          is_melee_w = weapon_groups::is_melee_type_id(type_id)
                    || weapon_groups::is_melee_def_id(def_id)
                    || weapon_groups::is_spy_knife_def_id(def_id);
        }
        ctx.melee_equipped = is_melee_w;
      }

      if (target_player && target_player->get_lifestate() == 1 && !target_player->is_dormant()) {
        Vec3 tpos = target_player->get_origin();
        ctx.have_target = true;
        ctx.target_index = target_player->get_index();
        ctx.target_pos[0] = tpos.x; ctx.target_pos[1] = tpos.y; ctx.target_pos[2] = tpos.z;
      }

      nav::navbot::BotOutput bo{};
      bool tick_ok = nav::navbot::Tick(ctx, &bo);
      if (bo.task != g_last_task) {
        print("navbot: task -> %s\n", TaskKindToString(bo.task));
        g_last_task = bo.task;
      }
      if (tick_ok && bo.have_waypoint) {
        float dx = bo.waypoint[0] - me.x;
        float dy = bo.waypoint[1] - me.y;
        float dist2 = dx*dx + dy*dy;

        float desired_yaw = std::atan2(dy, dx) * (180.0f / (float)M_PI);
        float cur_yaw = user_cmd->view_angles.y;
        float dyaw = ang_norm(desired_yaw - cur_yaw);
        float rad = dyaw * (float)M_PI / 180.0f;

        bool forward_clear = true;
        float speed = forward_clear ? 420.0f : 220.0f;
        float fwd = std::cos(rad) * speed;
        float side = -std::sin(rad) * speed;
        if (!forward_clear) fwd *= 0.35f;

        user_cmd->forwardmove = clampf(fwd, -450.0f, 450.0f);
        user_cmd->sidemove = clampf(side, -450.0f, 450.0f);

        if (bo.perform_crouch_jump) {
          user_cmd->buttons |= IN_DUCK;
          if (localplayer->get_ground_entity()) {
            user_cmd->buttons |= IN_JUMP;
          }
        }

        result.move_set = true;
        result.orig_view = orig_view;
        result.orig_forward = orig_forward;
        result.orig_side = orig_side;

        bool manual_shooting_now = ((user_cmd->buttons & IN_ATTACK) != 0) && !is_shooting;
        if (config.nav.look_at_path && !is_shooting && !manual_shooting_now) {
          Vec3 eye = localplayer->get_shoot_pos();
          float dy2 = bo.waypoint[1] - eye.y;
          float dx2 = bo.waypoint[0] - eye.x;
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

      // replace it with actual autoweapon logic
      // {
      //   if (g_weapon_switch_cooldown > 0) {
      //     --g_weapon_switch_cooldown;
      //   }

      //   int desired_slot = 1;
      //   const float melee_range = 120.0f;
      //   bool melee_close = false;
      //   if (target_player && target_player->get_lifestate() == 1 && !target_player->is_dormant()) {
      //     Vec3 tpos = target_player->get_origin();
      //     float dxm = tpos.x - me.x;
      //     float dym = tpos.y - me.y;
      //     float dzm = tpos.z - me.z;
      //     float d2 = dxm*dxm + dym*dym + dzm*dzm;
      //     melee_close = (d2 <= (melee_range * melee_range));
      //   }
      //   if (melee_close) desired_slot = 3;

      //   if (desired_slot != g_last_weapon_slot_selected && g_weapon_switch_cooldown == 0) {
      //     user_cmd->weapon_select = desired_slot;
      //     user_cmd->weapon_subtype = 0;
      //     g_last_weapon_slot_selected = desired_slot;
      //     g_weapon_switch_cooldown = 12;
      //     print("navbot: slot -> %d\n", desired_slot);
      //   }
      // }
    }
  }

  return result;
}

} // namespace

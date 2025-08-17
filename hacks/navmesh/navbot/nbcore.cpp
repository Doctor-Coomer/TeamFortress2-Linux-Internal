// melody 17/aug/2025

#include "nbcore.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <unordered_set>
#include <limits>

#include "../navengine.hpp"
#include "../navparser.hpp"
#include "../pathfinder.hpp"
#include "../tfnav_flags.hpp"

namespace nav { namespace navbot {

struct RoamState {
  uint32_t goal_id = 0;
  std::vector<uint32_t> path_ids;
  size_t next_index = 0;
  int last_plan_tick = 0;

  float last_wp_d2 = 0.f;
  int last_progress_tick = 0;
  int replan_cooldown_until = 0; 

  std::deque<uint32_t> visited_fifo;
  std::unordered_set<uint32_t> visited_set;
  int last_visited_clear_tick = 0;
  int consecutive_pick_failures = 0;
  uint32_t last_area_id = 0;

  bool antistuck_active = false;
  int antistuck_until_tick = 0;
  float antistuck_baseline_d2 = 0.f;
  int antistuck_attempts = 0;
  int antistuck_cooldown_until = 0;

  float last_pos[3] = {0.f, 0.f, 0.f};
  int last_pos_tick = 0;
  bool last_pos_valid = false;
  int teleport_cooldown_until = 0;
};

static RoamState g_roam;

static inline float dist2_2d(float ax, float ay, float bx, float by) {
  float dx = ax - bx, dy = ay - by;
  return dx*dx + dy*dy;
}

static constexpr size_t kVisitedCap = 512;
static constexpr float kMinRoamDist2 = 500.0f * 500.0f;   
static constexpr float kFarDist2     = 2000.0f * 2000.0f; 
static constexpr float kMidMinDist2  = 1000.0f * 1000.0f; 
static constexpr float kTeleportMinDist2 = 700.0f * 700.0f; 
static constexpr int   kTeleportMaxDt    = 6;               
static constexpr int   kTeleportCooldown = 66;

static inline void visited_clear() {
  g_roam.visited_fifo.clear();
  g_roam.visited_set.clear();
}

static inline void visited_add(uint32_t id) {
  if (!id) return;
  if (g_roam.visited_set.find(id) != g_roam.visited_set.end()) return;
  if (g_roam.visited_fifo.size() >= kVisitedCap) {
    uint32_t old = g_roam.visited_fifo.front();
    g_roam.visited_fifo.pop_front();
    g_roam.visited_set.erase(old);
  }
  g_roam.visited_fifo.push_back(id);
  g_roam.visited_set.insert(id);
}

static inline bool area_disallowed_for_goal(const nav::Area& a) {
  using namespace nav::tfnav;
  if ((a.tf_attribute_flags & kTFBadGoalMask) != 0) return true;
  if ((a.attributes & NAV_MESH_NAV_BLOCKER) != 0) return true;
  return false;
}

static const nav::Area* find_nearest_area_2d(float me_x, float me_y, float me_z) {
  const nav::Mesh* mesh = nav::GetMesh();
  if (!mesh || mesh->areas.empty()) return nullptr;
  const nav::Area* best = nullptr;
  float best_d2 = std::numeric_limits<float>::max();
  for (const auto& a : mesh->areas) {
    if (area_disallowed_for_goal(a)) continue;
    float c[3]; nav::path::GetAreaCenter(&a, c);
    // Height reachability: skip areas whose floor is too far above us to climb/jump.
    float z0 = a.nw[2], z1 = a.se[2], z2 = a.ne_z, z3 = a.sw_z;
    float min_z = std::min(std::min(z0, z1), std::min(z2, z3));
    const float kJumpHeight = 72.0f;
    const float kZSlop = 18.0f;
    if ((min_z - me_z) > (kJumpHeight + kZSlop)) continue;
    float d2 = dist2_2d(me_x, me_y, c[0], c[1]);
    if (d2 < best_d2) { best_d2 = d2; best = &a; }
  }
  return best;
}

static uint32_t pick_far_goal_from_here(const nav::Area* start, float me_x, float me_y, float me_z) {
  if (!start) return 0;
  const nav::Mesh* mesh = nav::GetMesh();
  if (!mesh || mesh->areas.empty()) return 0;

  float me2[3] = {me_x, me_y, me_z};
  uint32_t best_nonvisited_id = 0;
  float best_nonvisited_d2 = -1.f;
  uint32_t best_any_id = 0;
  float best_any_d2 = -1.f;
  for (const auto& a : mesh->areas) {
    if (a.id == start->id) continue;
    if (area_disallowed_for_goal(a)) continue;
    float c[3]; nav::path::GetAreaCenter(&a, c);
    float d2 = dist2_2d(me2[0], me2[1], c[0], c[1]);
    if (d2 > best_any_d2) { best_any_d2 = d2; best_any_id = a.id; }
    if (g_roam.visited_set.find(a.id) == g_roam.visited_set.end()) {
      if (d2 > best_nonvisited_d2) { best_nonvisited_d2 = d2; best_nonvisited_id = a.id; }
    }
  }
  return best_nonvisited_id ? best_nonvisited_id : best_any_id;
}

static bool plan_path(uint32_t start_id, uint32_t end_id);
static bool pick_new_goal_randomized(const nav::Area* start, float me_x, float me_y, float me_z, int cmd_number) {
  if (!start) return false;
  const nav::Mesh* mesh = nav::GetMesh();
  if (!mesh || mesh->areas.empty()) return false;

  float me2[3] = {me_x, me_y, me_z};

  struct Cand { const nav::Area* a; float d2; };
  std::vector<Cand> valid;
  valid.reserve(mesh->areas.size());
  for (const auto& a : mesh->areas) {
    if (a.id == start->id) continue;
    if (area_disallowed_for_goal(a)) continue;
    if (g_roam.visited_set.find(a.id) != g_roam.visited_set.end()) continue;
    float c[3]; nav::path::GetAreaCenter(&a, c);
    float d2 = dist2_2d(me2[0], me2[1], c[0], c[1]);
    if (d2 < kMinRoamDist2) continue;
    valid.push_back(Cand{&a, d2});
  }

  std::vector<Cand> any_allowed;
  if (valid.empty()) {
    any_allowed.reserve(mesh->areas.size());
    for (const auto& a : mesh->areas) {
      if (a.id == start->id) continue;
      if (area_disallowed_for_goal(a)) continue;
      float c[3]; nav::path::GetAreaCenter(&a, c);
      float d2 = dist2_2d(me2[0], me2[1], c[0], c[1]);
      if (d2 < kMinRoamDist2) continue;
      any_allowed.push_back(Cand{&a, d2});
    }
  }

  auto build_bucket_order = [&](const std::vector<Cand>& src) {
    std::vector<const nav::Area*> out;
    out.reserve(src.size());
    // Far bucket
    for (const auto& it : src) if (it.d2 > kFarDist2) out.push_back(it.a);
    if (out.empty()) {
      // Medium bucket
      for (const auto& it : src) if (it.d2 > kMidMinDist2 && it.d2 <= kFarDist2) out.push_back(it.a);
      if (out.empty()) {
        for (const auto& it : src) out.push_back(it.a); // any
      }
    }
    // Shuffle to avoid deterministic back-and-forth
    for (size_t i = out.size(); i > 1; --i) {
      size_t j = static_cast<size_t>(std::rand()) % i; // [0, i)
      std::swap(out[i-1], out[j]);
    }
    return out;
  };

  auto ordered = build_bucket_order(valid.empty() ? any_allowed : valid);

  for (const nav::Area* tgt : ordered) {
    if (plan_path(start->id, tgt->id)) {
      g_roam.goal_id = tgt->id;
      g_roam.last_plan_tick = cmd_number;
      g_roam.last_wp_d2 = 0.f;
      g_roam.last_progress_tick = cmd_number;
      g_roam.consecutive_pick_failures = 0;
      return true;
    }
  }

  uint32_t far_id = pick_far_goal_from_here(start, me_x, me_y, me_z);
  if (far_id && plan_path(start->id, far_id)) {
    g_roam.goal_id = far_id;
    g_roam.last_plan_tick = cmd_number;
    g_roam.last_wp_d2 = 0.f;
    g_roam.last_progress_tick = cmd_number;
    g_roam.consecutive_pick_failures = 0;
    return true;
  }

  g_roam.replan_cooldown_until = cmd_number + 66; // ~1s backoff
  if (++g_roam.consecutive_pick_failures >= 4) {
    visited_clear();
    g_roam.consecutive_pick_failures = 0;
  }
  return false;
}

static bool plan_path(uint32_t start_id, uint32_t end_id) {
  g_roam.path_ids.clear();
  g_roam.next_index = 0;
  if (!start_id || !end_id || start_id == end_id) return false;
  std::vector<const nav::Area*> path_areas;
  float total_cost = 0.0f;
  int rc = nav::path::FindPath(start_id, end_id, path_areas, &total_cost);
  if (rc == 0 || rc == 2) {
    g_roam.path_ids.reserve(path_areas.size());
    for (auto* a : path_areas) g_roam.path_ids.push_back(a->id);
    if (!g_roam.path_ids.empty() && g_roam.path_ids[0] == start_id) {
      g_roam.next_index = 1;
    }
    return !g_roam.path_ids.empty();
  }
  return false;
}

void Reset() {
  g_roam = RoamState{};
  nav::Visualizer_ClearPath();
}

bool Tick(float me_x, float me_y, float me_z, int cmd_number, RoamOutput* out) {
  if (out) *out = RoamOutput{};
  if (!nav::IsLoaded()) return false;

  const nav::Area* cur_area = nav::FindBestAreaAtPosition(me_x, me_y, me_z);
  bool teleported = false;
  if (g_roam.last_pos_valid) {
    int dt = cmd_number - g_roam.last_pos_tick;
    if (dt > 0 && dt <= kTeleportMaxDt && cmd_number >= g_roam.teleport_cooldown_until) {
      float d2 = dist2_2d(me_x, me_y, g_roam.last_pos[0], g_roam.last_pos[1]);
      if (d2 > kTeleportMinDist2) teleported = true;
    }
  }
  if (teleported && cur_area) {
    visited_clear();
    g_roam.consecutive_pick_failures = 0;
    g_roam.antistuck_active = false;
    g_roam.antistuck_attempts = 0;
    g_roam.antistuck_cooldown_until = cmd_number + 132;

    if (g_roam.goal_id) {
      if (plan_path(cur_area->id, g_roam.goal_id)) {
        g_roam.last_plan_tick = cmd_number;
        g_roam.last_wp_d2 = 0.f;
        g_roam.last_progress_tick = cmd_number;
        if (!g_roam.path_ids.empty() && g_roam.path_ids[0] == cur_area->id) {
          g_roam.next_index = 1;
        } else {
          g_roam.next_index = 0;
        }
      } else {
        g_roam.goal_id = 0;
        g_roam.path_ids.clear();
        g_roam.next_index = 0;
        g_roam.replan_cooldown_until = cmd_number;
      }
    }
    if (!g_roam.goal_id) {
      (void)pick_new_goal_randomized(cur_area, me_x, me_y, me_z, cmd_number);
    }
    g_roam.teleport_cooldown_until = cmd_number + kTeleportCooldown;
  }
  if (g_roam.last_visited_clear_tick == 0 || (cmd_number - g_roam.last_visited_clear_tick) > 11880) {
    visited_clear();
    g_roam.consecutive_pick_failures = 0;
    g_roam.last_visited_clear_tick = cmd_number;
  }

  if (cur_area) {
    if (cur_area->id != g_roam.last_area_id) {
      visited_add(cur_area->id);
      g_roam.last_area_id = cur_area->id;
    }
  } else {
    if (g_roam.next_index < g_roam.path_ids.size()) {
      const nav::Area* next_a = nav::path::GetAreaById(g_roam.path_ids[g_roam.next_index]);
      if (next_a && out) {
        float c[3]; nav::path::GetAreaCenter(next_a, c);
        out->have_waypoint = true;
        out->waypoint[0] = c[0]; out->waypoint[1] = c[1]; out->waypoint[2] = c[2];
        out->goal_area_id = g_roam.goal_id;
        out->next_index = g_roam.next_index;
      }
    }
    if (out && !out->have_waypoint) {
      const nav::Area* near_a = find_nearest_area_2d(me_x, me_y, me_z);
      if (near_a) {
        float c[3]; nav::path::GetAreaCenter(near_a, c);
        out->have_waypoint = true;
        out->waypoint[0] = c[0]; out->waypoint[1] = c[1]; out->waypoint[2] = c[2];
        out->goal_area_id = 0;
        out->next_index = 0;
      }
    }
    g_roam.last_pos[0] = me_x; g_roam.last_pos[1] = me_y; g_roam.last_pos[2] = me_z;
    g_roam.last_pos_tick = cmd_number;
    g_roam.last_pos_valid = true;
    nav::Visualizer_SetPath(g_roam.path_ids, g_roam.next_index, g_roam.goal_id);
    return out && out->have_waypoint;
  }

  if (!g_roam.goal_id && cmd_number >= g_roam.replan_cooldown_until) {
    if (g_roam.last_plan_tick == 0 || (cmd_number - g_roam.last_plan_tick) >= 132) {
      pick_new_goal_randomized(cur_area, me_x, me_y, me_z, cmd_number);
    }
  }

  if (g_roam.goal_id && (g_roam.path_ids.empty() || g_roam.next_index >= g_roam.path_ids.size())) {
    if (cur_area->id == g_roam.goal_id) {
      g_roam.goal_id = 0;
      g_roam.path_ids.clear();
      g_roam.next_index = 0;
      g_roam.last_plan_tick = cmd_number;
    } else {
      if (plan_path(cur_area->id, g_roam.goal_id)) {
        g_roam.last_plan_tick = cmd_number;
        g_roam.last_wp_d2 = 0.f;
        g_roam.last_progress_tick = cmd_number;
        g_roam.consecutive_pick_failures = 0;
      } else {
        g_roam.goal_id = 0;
        g_roam.replan_cooldown_until = cmd_number + 66;
        if (++g_roam.consecutive_pick_failures >= 4) {
          visited_clear();
          g_roam.consecutive_pick_failures = 0;
        }
      }
    }
  }

  if (g_roam.next_index < g_roam.path_ids.size()) {
    const nav::Area* next_a = nav::path::GetAreaById(g_roam.path_ids[g_roam.next_index]);
    if (next_a) {
      float c[3]; nav::path::GetAreaCenter(next_a, c);
      float d2 = dist2_2d(me_x, me_y, c[0], c[1]);
      if (d2 < (40.0f*40.0f)) {
        g_roam.next_index++;
        g_roam.last_wp_d2 = 0.f;
        g_roam.last_progress_tick = cmd_number;
      } else {
        if (g_roam.last_wp_d2 <= 0.f) {
          g_roam.last_wp_d2 = d2;
          g_roam.last_progress_tick = cmd_number;
        } else {
          if (d2 + 25.0f < g_roam.last_wp_d2) {
            g_roam.last_wp_d2 = d2;
            g_roam.last_progress_tick = cmd_number;
            if (g_roam.antistuck_active) {
              g_roam.antistuck_active = false;
              g_roam.antistuck_attempts = 0;
              g_roam.antistuck_cooldown_until = cmd_number + 132;
            }
          } else {
            if (!g_roam.antistuck_active && cmd_number >= g_roam.antistuck_cooldown_until && (cmd_number - g_roam.last_progress_tick) > 66) {
              g_roam.antistuck_active = true;
              g_roam.antistuck_until_tick = cmd_number + 18;
              g_roam.antistuck_baseline_d2 = d2;
            }
            if (g_roam.antistuck_active && cmd_number >= g_roam.antistuck_until_tick) {
              g_roam.antistuck_active = false;
              g_roam.antistuck_attempts++;
              bool progressed = (d2 + 25.0f < g_roam.antistuck_baseline_d2);
              if (!progressed) {
                if (g_roam.goal_id) {
                  uint32_t failed_goal = g_roam.goal_id;
                  visited_add(failed_goal);
                  if (next_a) visited_add(next_a->id);
                  g_roam.goal_id = 0;
                  g_roam.path_ids.clear();
                  g_roam.next_index = 0;
                  g_roam.replan_cooldown_until = cmd_number + 66;
                  g_roam.consecutive_pick_failures = 0;
                  g_roam.antistuck_cooldown_until = cmd_number + 198;
                }
              } else {
                g_roam.antistuck_attempts = 0;
                g_roam.antistuck_cooldown_until = cmd_number + 132;
                g_roam.last_progress_tick = cmd_number;
              }
            }
          }
        }
      }
    }
  }

  if (g_roam.next_index < g_roam.path_ids.size()) {
    const nav::Area* next_a = nav::path::GetAreaById(g_roam.path_ids[g_roam.next_index]);
    if (next_a) {
      float c[3]; nav::path::GetAreaCenter(next_a, c);
      if (out) {
        out->have_waypoint = true;
        out->waypoint[0] = c[0]; out->waypoint[1] = c[1]; out->waypoint[2] = c[2];
        out->goal_area_id = g_roam.goal_id;
        out->next_index = g_roam.next_index;
        out->perform_crouch_jump = g_roam.antistuck_active;
      }
    }
  }

  g_roam.last_pos[0] = me_x; g_roam.last_pos[1] = me_y; g_roam.last_pos[2] = me_z;
  g_roam.last_pos_tick = cmd_number;
  g_roam.last_pos_valid = true;
  nav::Visualizer_SetPath(g_roam.path_ids, g_roam.next_index, g_roam.goal_id);
  return out && out->have_waypoint;
}

}} // namespace nav::navbot

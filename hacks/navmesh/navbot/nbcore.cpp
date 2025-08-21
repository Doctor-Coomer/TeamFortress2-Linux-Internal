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
#include "../../../classes/entity.hpp"
#include "../../../interfaces/entity_list.hpp"
// Needed for Player method calls (team, lifestate, origin)
#include "../../../classes/player.hpp"
// Ray/visibility for LOS-based snipe goal search
#include "../../../interfaces/engine_trace.hpp"

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

struct ChaseState {
  bool active = false;
  int last_target_idx = 0;
  float last_goal[3] = {0.f, 0.f, 0.f};
};

static ChaseState g_chase;
static TaskKind g_task = TaskKind::Roam;
// Preserve last non-roam task and tolerate brief target loss
static TaskKind g_prev_task = TaskKind::Roam;
static int g_target_grace_until = 0; // cmd_number until which we pretend target still exists

struct SnipeState {
  bool anchor_valid = false;
  float anchor[3] = {0.f, 0.f, 0.f};
  int   last_anchor_tick = 0;
  int   min_hold_until = 0;
};
static SnipeState g_snipe;

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
  g_chase = ChaseState{};
  g_task = TaskKind::Roam;
  g_snipe = SnipeState{};
  nav::Visualizer_ClearPath();
}

uint32_t CurrentGoalArea() {
  return g_roam.goal_id;
}

void ClearGoal() {
  g_roam.goal_id = 0;
  g_roam.path_ids.clear();
  g_roam.next_index = 0;
  g_roam.last_plan_tick = 0;
  g_roam.last_wp_d2 = 0.f;
  g_roam.last_progress_tick = 0;
  nav::Visualizer_ClearPath();
}

bool PlanToPositionFrom(float me_x, float me_y, float me_z,
                        float goal_x, float goal_y, float goal_z,
                        int cmd_number) {
  if (!nav::IsLoaded()) return false;

  const nav::Area* start = nav::FindBestAreaAtPosition(me_x, me_y, me_z);
  if (!start) start = find_nearest_area_2d(me_x, me_y, me_z);
  if (!start) return false;

  const nav::Area* goal = nav::FindBestAreaAtPosition(goal_x, goal_y, goal_z);
  if (!goal) goal = find_nearest_area_2d(goal_x, goal_y, goal_z);
  if (!goal) return false;

  // Skip disallowed goals (blockers/bad areas). Find a nearby allowed candidate by 2D nearest.
  if (area_disallowed_for_goal(*goal)) {
    goal = find_nearest_area_2d(goal_x, goal_y, goal_z);
    if (!goal || area_disallowed_for_goal(*goal)) return false;
  }

  if (plan_path(start->id, goal->id)) {
    g_roam.goal_id = goal->id;
    g_roam.last_plan_tick = cmd_number;
    g_roam.last_wp_d2 = 0.f;
    g_roam.last_progress_tick = cmd_number;
    if (!g_roam.path_ids.empty() && g_roam.path_ids[0] == start->id) {
      g_roam.next_index = 1;
    } else {
      g_roam.next_index = 0;
    }
    nav::Visualizer_SetPath(g_roam.path_ids, g_roam.next_index, g_roam.goal_id);
    return true;
  }

  return false;
}

bool Tick(const BotContext& ctx, BotOutput* out) {
  if (out) *out = BotOutput{};
  if (!nav::IsLoaded()) return false;

  // Snapshot inputs locally for minimal code churn
  const float me_x = ctx.me[0];
  const float me_y = ctx.me[1];
  const float me_z = ctx.me[2];
  const int   cmd_number = ctx.cmd_number;

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

  // -----------------
  // Task scheduler
  // -----------------
  g_task = TaskKind::Roam;
  // Target with grace period to avoid flicker when aimbot drops target for a few ticks
  bool have_target_now = ctx.have_target;
  if (have_target_now) {
    g_target_grace_until = cmd_number + 16; // ~0.25s at 66tps
  }
  bool had_combat_task = (g_prev_task == TaskKind::Snipe) || (g_prev_task == TaskKind::ChaseMelee);
  bool within_grace = (cmd_number < g_target_grace_until) && had_combat_task;
  bool have_target = have_target_now || within_grace;
  float tpos[3] = {0.f, 0.f, 0.f};
  int   tidx = 0;
  if (have_target_now) {
    tidx = ctx.target_index;
    tpos[0] = ctx.target_pos[0];
    tpos[1] = ctx.target_pos[1];
    tpos[2] = ctx.target_pos[2];
  } else if (within_grace) {
    tidx = g_chase.last_target_idx;
    tpos[0] = g_chase.last_goal[0];
    tpos[1] = g_chase.last_goal[1];
    tpos[2] = g_chase.last_goal[2];
  }

  // Snipe fallback target: if no aimbot/grace target, pick nearest enemy to stalk
  bool snipe_have_target = have_target;
  float snipe_tpos[3] = {tpos[0], tpos[1], tpos[2]};
  int   snipe_tidx = tidx;
  if (!snipe_have_target && ctx.scheduler_enabled && ctx.snipe_enabled && !ctx.melee_equipped && entity_list) {
    Player* me = entity_list->get_localplayer();
    if (me) {
      float best_d2_any = std::numeric_limits<float>::max();
      int best_idx = 0;
      float best_pos[3] = {0.f, 0.f, 0.f};
      int max_e = entity_list->get_max_entities();
      for (int i = 1; i <= max_e; ++i) {
        Player* p = entity_list->player_from_index(i);
        if (!p || p == me) continue;
        if (p->is_dormant()) continue;
        if (p->get_team() == me->get_team()) continue;
        if (p->get_lifestate() != 1) continue;
        if (p->is_invulnerable()) continue;
        Vec3 o = p->get_origin();
        float d2 = dist2_2d(me_x, me_y, o.x, o.y);
        if (d2 < best_d2_any) {
          best_d2_any = d2;
          best_idx = i;
          best_pos[0] = o.x; best_pos[1] = o.y; best_pos[2] = o.z;
        }
      }
      if (best_idx != 0) {
        snipe_have_target = true;
        snipe_tidx = best_idx;
        snipe_tpos[0] = best_pos[0]; snipe_tpos[1] = best_pos[1]; snipe_tpos[2] = best_pos[2];
      }
    }
  }

  // Distance to target for auto-melee gating (aimbot/grace target only)
  float dx_t = 0.f, dy_t = 0.f;
  float d2_t = std::numeric_limits<float>::max();
  if (have_target) {
    dx_t = tpos[0] - me_x; dy_t = tpos[1] - me_y;
    d2_t = dx_t*dx_t + dy_t*dy_t;
  }
  const float kAutoMeleeHU = 300.0f;
  const bool in_auto_melee_range = have_target && (d2_t <= (kAutoMeleeHU * kAutoMeleeHU));

  // Snipe-specific melee gating based on snipe fallback target if any
  float sdx_t = 0.f, sdy_t = 0.f;
  float s_d2_t = std::numeric_limits<float>::max();
  if (snipe_have_target) {
    sdx_t = snipe_tpos[0] - me_x; sdy_t = snipe_tpos[1] - me_y;
    s_d2_t = sdx_t*sdx_t + sdy_t*sdy_t;
  }
  const bool snipe_in_auto_melee_range = snipe_have_target && (s_d2_t <= (kAutoMeleeHU * kAutoMeleeHU));

  const bool snipe_possible = ctx.scheduler_enabled && ctx.snipe_enabled && snipe_have_target && !ctx.melee_equipped && !snipe_in_auto_melee_range;
  const bool chase_possible = ctx.scheduler_enabled && ctx.chase_enabled && have_target && (!ctx.chase_only_when_melee || ctx.melee_equipped);

  // Compute desired snipe ring point around target (preferred range)
  auto compute_snipe_goal = [&](float out[3]) {
    const float rx = me_x - snipe_tpos[0];
    const float ry = me_y - snipe_tpos[1];
    const float rz = me_z - snipe_tpos[2];
    float r2 = rx*rx + ry*ry + rz*rz;
    float r = r2 > 1e-3f ? std::sqrt(r2) : 0.f;
    float desired = ctx.snipe_preferred_range;
    if (desired < 100.f) desired = 100.f;
    float nx, ny, nz;
    if (r > 1e-3f) {
      nx = rx / r; ny = ry / r; nz = rz / r;
    } else {
      nx = 1.f; ny = 0.f; nz = 0.f; // arbitrary if overlapping
    }
    out[0] = snipe_tpos[0] + nx * desired;
    out[1] = snipe_tpos[1] + ny * desired;
    out[2] = snipe_tpos[2] + nz * 0.f; // keep roughly same height preference; let nav nearest choose floor
  };

  // Compute an angle-searched goal that has LOS to the target's head. Returns true on success.
  auto compute_snipe_goal_los = [&](float out[3]) -> bool {
    if (!engine_trace || !entity_list) return false;
    Player* me_ent = entity_list->get_localplayer();
    if (!me_ent) return false;
    Player* tgt = entity_list->player_from_index(snipe_tidx);
    if (!tgt) return false;

    // Target head position
    int head_bone = tgt->get_head_bone();
    Vec3 t_head = tgt->get_bone_pos(head_bone);
    t_head.z += 4.0f;

    // We'll sample around the target on multiple radii, including inside preferred range
    const float desired = std::max(100.0f, ctx.snipe_preferred_range);
    const float radii[] = {
      desired,
      desired * 1.25f,
      desired * 0.75f,
      desired * 1.5f,
      std::max(300.0f, desired * 0.6f),
      std::max(300.0f, desired * 0.4f)
    };
    // Base angle: from target to me
    float base_dx = me_x - snipe_tpos[0];
    float base_dy = me_y - snipe_tpos[1];
    float base_ang = std::atan2(base_dy, base_dx);

    // Eye height approximation when evaluating candidate LOS
    const float kEyeH = 64.0f;

    // Prepare trace filter to skip self
    trace_filter filter{};
    engine_trace->init_trace_filter(&filter, (void*)me_ent);

    bool found = false;
    float best_cost = std::numeric_limits<float>::max();
    float best_goal[3] = {0.f, 0.f, 0.f};

    // Prefer near-side angles first: sweep around the circle
    const int kSamples = 12;
    for (float r : radii) {
      for (int i = 0; i < kSamples; ++i) {
        float ang = base_ang + (static_cast<float>(i) * (2.0f * 3.14159265f / kSamples));
        float gx = snipe_tpos[0] + std::cos(ang) * r;
        float gy = snipe_tpos[1] + std::sin(ang) * r;
        float gz = snipe_tpos[2];

        // Candidate eye pos
        Vec3 start = {gx, gy, gz + kEyeH};
        Vec3 end = t_head;
        ray_t ray = engine_trace->init_ray(&start, &end);
        trace_t tr{};
        // Use a visibility-friendly mask including solids and hitboxes
        unsigned int mask = MASK_SOLID | CONTENTS_GRATE | CONTENTS_WINDOW | CONTENTS_IGNORE_NODRAW_OPAQUE | CONTENTS_HITBOX;
        engine_trace->trace_ray(&ray, mask, &filter, &tr);

        bool visible = (tr.fraction >= 0.99f) || (tr.entity == (void*)tgt);
        if (!visible) continue;

        // Cost: distance from our current pos + deviation from desired range
        float ddx = gx - me_x; float ddy = gy - me_y;
        float travel_cost = ddx*ddx + ddy*ddy; // 2D squared distance to minimize detours
        float range_dev = std::fabs(r - desired);
        float cost = travel_cost + (range_dev * range_dev);
        if (cost < best_cost) {
          best_cost = cost;
          best_goal[0] = gx; best_goal[1] = gy; best_goal[2] = gz;
          found = true;
        }
      }
      if (found) break; // accept first radius that yields LOS
    }

    if (found) {
      out[0] = best_goal[0]; out[1] = best_goal[1]; out[2] = best_goal[2];
      return true;
    }
    return false;
  };

  // Shared chase planner (melee). Returns true if it set a chase task/goal.
  auto try_chase_melee = [&]() -> bool {
    // Distance gate for melee chase
    float dx_d = tpos[0] - me_x, dy_d = tpos[1] - me_y;
    float d2_d = dx_d*dx_d + dy_d*dy_d;
    float max_d = ctx.chase_distance_max; if (max_d < 1.f) max_d = 1.f;
    if (d2_d <= (max_d * max_d)) {
      int dt = cmd_number - g_roam.last_plan_tick;
      float gdx = tpos[0] - g_chase.last_goal[0];
      float gdy = tpos[1] - g_chase.last_goal[1];
      float gdz = tpos[2] - g_chase.last_goal[2];
      float moved2 = gdx*gdx + gdy*gdy + gdz*gdz;
      float move_thresh = ctx.chase_replan_move_threshold; if (move_thresh < 1.f) move_thresh = 1.f;
      bool need_replan = (tidx != g_chase.last_target_idx) || (dt >= ctx.chase_repath_ticks) || (moved2 > (move_thresh * move_thresh));

      if (need_replan) {
        if (nav::navbot::PlanToPositionFrom(me_x, me_y, me_z, tpos[0], tpos[1], tpos[2], cmd_number)) {
          g_chase.last_target_idx = tidx;
          g_roam.last_plan_tick = cmd_number;
          g_chase.last_goal[0] = tpos[0]; g_chase.last_goal[1] = tpos[1]; g_chase.last_goal[2] = tpos[2];
          g_chase.active = true;
          g_task = TaskKind::ChaseMelee;
          return true;
        }
      } else {
        g_chase.active = true; // continue chasing with current goal
        g_task = TaskKind::ChaseMelee;
        return true;
      }
    } else if (g_chase.active) {
      nav::navbot::ClearGoal();
      g_chase.active = false;
    }
    return false;
  };

  if (ctx.melee_equipped) {
    // With melee out: do not snipe.
    if (chase_possible) {
      (void)try_chase_melee();
      if (g_chase.active) {
        g_task = TaskKind::ChaseMelee;
      } else {
        g_task = TaskKind::Roam;
      }
    } else {
      if (g_chase.active) { nav::navbot::ClearGoal(); g_chase.active = false; }
      g_task = TaskKind::Roam;
    }
  } else if (snipe_possible) {
    float cand[3];
    bool got_los = compute_snipe_goal_los(cand);
    if (!got_los) {
      compute_snipe_goal(cand);
    }

    const int kMinHoldTicks = 528; // ~8s at 66tps
    float move_thresh_base = ctx.snipe_replan_move_threshold; if (move_thresh_base < 1.f) move_thresh_base = 1.f;
    const float anchor_soft = move_thresh_base * 1.75f;

    bool target_changed = (snipe_tidx != g_chase.last_target_idx);
    if (!g_snipe.anchor_valid || target_changed) {
      g_snipe.anchor_valid = true;
      g_snipe.anchor[0] = cand[0]; g_snipe.anchor[1] = cand[1]; g_snipe.anchor[2] = cand[2];
      g_snipe.last_anchor_tick = cmd_number;
      g_snipe.min_hold_until = cmd_number + kMinHoldTicks;
    } else {
      float adx = cand[0] - g_snipe.anchor[0];
      float ady = cand[1] - g_snipe.anchor[1];
      float adz = cand[2] - g_snipe.anchor[2];
      float moved2_anchor = adx*adx + ady*ady + adz*adz;
      bool can_change = (cmd_number >= g_snipe.min_hold_until);
      if (can_change && (moved2_anchor > (anchor_soft * anchor_soft))) {
        g_snipe.anchor[0] = cand[0]; g_snipe.anchor[1] = cand[1]; g_snipe.anchor[2] = cand[2];
        g_snipe.last_anchor_tick = cmd_number;
        g_snipe.min_hold_until = cmd_number + kMinHoldTicks;
      }
    }

    float goal[3] = { g_snipe.anchor[0], g_snipe.anchor[1], g_snipe.anchor[2] };

    int dt = cmd_number - g_roam.last_plan_tick;
    float gdx = goal[0] - g_chase.last_goal[0];
    float gdy = goal[1] - g_chase.last_goal[1];
    float gdz = goal[2] - g_chase.last_goal[2];
    float moved2 = gdx*gdx + gdy*gdy + gdz*gdz;
    float move_thresh = ctx.snipe_replan_move_threshold; if (move_thresh < 1.f) move_thresh = 1.f;

    int min_repath_ticks = std::max(ctx.snipe_repath_ticks, kMinHoldTicks);
    bool need_replan = (snipe_tidx != g_chase.last_target_idx) || (dt >= min_repath_ticks);

    if (need_replan) {
      if (nav::navbot::PlanToPositionFrom(me_x, me_y, me_z, goal[0], goal[1], goal[2], cmd_number)) {
        g_chase.last_target_idx = snipe_tidx;
        g_roam.last_plan_tick = cmd_number;
        g_chase.last_goal[0] = goal[0]; g_chase.last_goal[1] = goal[1]; g_chase.last_goal[2] = goal[2];
        g_chase.active = true;
        g_task = TaskKind::Snipe;
      } else {
        // Keep intent to Snipe even if a replan fails this tick (avoid flicker to Roam)
        g_task = TaskKind::Snipe;
      }
    } else {
      g_chase.active = true;
      g_task = TaskKind::Snipe;
    }
  } else if (chase_possible) {
    (void)try_chase_melee();
    if (g_chase.active) {
      g_task = TaskKind::ChaseMelee;
    } else {
      // Keep intent rather than flicker to Roam if we just failed a plan this tick
      g_task = (g_prev_task == TaskKind::ChaseMelee) ? TaskKind::ChaseMelee : TaskKind::Roam;
    }
  } else if (g_chase.active) {
    nav::navbot::ClearGoal();
    g_chase.active = false;
    g_task = TaskKind::Roam;
  } else {
    // No snipe/chase path set this tick. If a target exists (or within grace), keep previous combat task label
    if (have_target) {
      if (g_prev_task == TaskKind::Snipe || g_prev_task == TaskKind::ChaseMelee) {
        g_task = g_prev_task;
      }
    }
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
    if (out) out->task = g_task;
    return out && out->have_waypoint;
  }

  // If we're actually Roaming, ensure we have a roam goal
  if (g_task == TaskKind::Roam && !g_chase.active) {
    if (!g_roam.goal_id && cmd_number >= g_roam.replan_cooldown_until) {
      if (g_roam.last_plan_tick == 0 || (cmd_number - g_roam.last_plan_tick) >= 132) {
        pick_new_goal_randomized(cur_area, me_x, me_y, me_z, cmd_number);
      }
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
        out->task = g_task;
      }
    }
  }

  g_roam.last_pos[0] = me_x; g_roam.last_pos[1] = me_y; g_roam.last_pos[2] = me_z;
  g_roam.last_pos_tick = cmd_number;
  g_roam.last_pos_valid = true;
  nav::Visualizer_SetPath(g_roam.path_ids, g_roam.next_index, g_roam.goal_id);
  if (out) out->task = g_task;
  g_prev_task = g_task;
  return out && out->have_waypoint;
}

}} // namespace nav::navbot

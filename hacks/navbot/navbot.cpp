#include "navmesh.hpp"

#include <string>
#include <cmath>
#include <algorithm>

#include "micropather/micropather.h"
#include "micropather/nav_graph.hpp"

#include "../../math.hpp"

#include "../../gui/config.hpp"

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/global_vars.hpp"

#include "../../classes/player.hpp"
#include "../../classes/capture_flag.hpp"
#include "../../classes/team_objective_resource.hpp"

#include "../../entity_cache.hpp"

#include "../../print.hpp"

struct PathState {
  static float CurTimeSeconds() { return global_vars ? global_vars->curtime : 0.0f; }
  static void Clear() { path = Path{}; }
  static void ResetCurrent() {
    path.goal_id = 0;
    path.path_ids.clear();
    path.next_index = 0;
  }
};


static bool make_path(unsigned int area_start_id, unsigned int area_end_id);

static bool areas_connected(Area* a, Area* b) {
  if (!a || !b) return false;
  for (int dir = 0; dir < 4; ++dir) {
    for (uint32_t id : a->connections[dir]) {
      if (id == b->id) return true;
    }
  }
  return false;
}

static bool set_path_from_to(Area* from, Area* goal) {
  if (!from || !goal) return false;
  if (!make_path(from->id, goal->id)) return false;
  path.goal_id = goal->id;
  path.next_index = (path.path_ids.size() > 1 && path.path_ids[0] == from->id) ? 1u : 0u;
  return true;
}

static Area* resolve_target_area_for(const Vec3& target_location) {
  Area* target_area = mesh.find_nearest_area_2d(target_location);
  if (target_area == nullptr) {
    target_area = mesh.best_area_from_xyz(target_location);
  }
  if (target_area && target_area->is_disallowed_for_goal()) {
    Area* tmp = target_area;
    Area* best_allowed = nullptr;
    float best_d2 = std::numeric_limits<float>::max();
    Vec3 tc = tmp->center();
    for (int d = 0; d < 4; ++d) {
      for (uint32_t nid : tmp->connections[d]) {
        Area* n = mesh.id_to_area(nid);
        if (!n || n->is_disallowed_for_goal()) continue;
        float d2 = distance_squared_2d(tc, n->center());
        if (d2 < best_d2) { best_d2 = d2; best_allowed = n; }
      }
    }
    if (best_allowed) target_area = best_allowed;
  }
  return target_area;
}

static inline void FinishPathIfOnGoal(Area* current_area) {
  if (current_area && current_area->id == path.goal_id) {
    PathState::ResetCurrent();
  }
}

static inline bool advance_if_on_next_area(Area* current_area, Area*& next_area) {
  if (current_area && next_area == current_area) {
    path.next_index++;
    if (path.next_index >= path.path_ids.size()) {
      PathState::ResetCurrent();
      next_area = nullptr;
      return true; // finished
    }
    next_area = mesh.id_to_area(path.path_ids[path.next_index]);
  }
  return false;
}

static bool make_path(unsigned int area_start_id, unsigned int area_end_id) {
  path.path_ids.clear();
  path.next_index = 0;
  if (!area_start_id || !area_end_id || area_start_id == area_end_id) return false;
  
  Area* a = mesh.id_to_area(area_start_id);
  Area* b = mesh.id_to_area(area_end_id);
  if (a == nullptr || b == nullptr) return false;
  
  static micropather::MicroPather* path_solver = new micropather::MicroPather(new NavGraph());
  MP_VECTOR<void*> states;
  float cost = 0.0f;
  int rc = path_solver->Solve((void*)a, (void*)b, &states, &cost);
  if (rc == micropather::MicroPather::SOLVED || rc == micropather::MicroPather::START_END_SAME) {
    path.path_ids.reserve(states.size());
    for (void* s : states) {
      path.path_ids.push_back(((Area*)s)->id);
    }
  }

  if (!path.path_ids.empty() && path.path_ids[0] == area_start_id) {
    path.next_index = 1;
  }
  
  return !path.path_ids.empty();
}

namespace jobs {
  enum class Kind { NoJob, Capture, Snipe, Roam };

  struct JobResult {
    Kind kind = Kind::NoJob;
    bool has_target = false;
    Vec3 target;
  };

  enum class GameMode { UNKNOWN, CTF, CP, PL, PLR, KOTH };

  static GameMode determine_game_mode(const char* level_name) {
    if (!level_name) return GameMode::UNKNOWN;
    std::string_view name(level_name);
    size_t under = name.find('_');
    std::string_view mode = (under == std::string_view::npos) ? name : name.substr(0, under);
    if (mode == "ctf")  return GameMode::CTF;
    if (mode == "cp")   return GameMode::CP;
    if (mode == "pl")   return GameMode::PL;
    if (mode == "plr")  return GameMode::PLR;
    if (mode == "koth") return GameMode::KOTH;
    return GameMode::UNKNOWN;
  }

  static Vec3 ctf_target_location(Player* localplayer) {
    Vec3 target_location{};
    CaptureFlag* enemy_intelligence = nullptr;
    CaptureFlag* team_intelligence  = nullptr;

    for (unsigned int i = 0; i < entity_cache[class_id::CAPTURE_FLAG].size(); ++i) {
      Entity* entity = entity_cache[class_id::CAPTURE_FLAG].at(i);
      if (entity == nullptr) continue;
  CaptureFlag* temp_flag = (CaptureFlag*)entity;
      if (entity->get_team() != localplayer->get_team()) {
        enemy_intelligence = temp_flag;
        continue;
      }
      if (entity->get_team() == localplayer->get_team()) {
        team_intelligence = temp_flag;
        continue;
      }
    }

    CaptureFlag* target_intelligence = nullptr;
    if (enemy_intelligence != nullptr && team_intelligence != nullptr) {
      if (team_intelligence->get_owner_entity() != nullptr || team_intelligence->get_status() == flag_status::DROPPED) {
        target_intelligence = team_intelligence;
      } else if (enemy_intelligence->get_owner_entity() != localplayer) {
        target_intelligence = enemy_intelligence;
      } else if (enemy_intelligence->get_owner_entity() == localplayer && team_intelligence->get_status() == flag_status::HOME) {
        target_intelligence = team_intelligence;
      }

      if (target_intelligence != nullptr)
        target_location = target_intelligence->get_origin();
    }
    return target_location;
  }

  static Vec3 koth_target_location(Player* localplayer) {
    Vec3 target_location{};
    TeamObjectiveResource* objective_resource = nullptr;
    for (unsigned int i = 0; i < entity_cache[class_id::OBJECTIVE_RESOURCE].size(); ++i) {
      Entity* entity = entity_cache[class_id::OBJECTIVE_RESOURCE].at(i);
      if (entity == nullptr) continue;
      objective_resource = (TeamObjectiveResource*)entity;
      break;
    }
    if (!objective_resource) return target_location;

    // Only consider a CP as a valid if
    // not locked
    // isnt already seized by my team
    // seized by enemy or neutral
    int count = objective_resource->get_num_control_points();
    if (count <= 0) return target_location;

    float best_d2 = std::numeric_limits<float>::max();
    Vec3 me = localplayer->get_origin();
    for (int idx = 0; idx < count; ++idx) {
      if (objective_resource->is_locked(idx)) continue;
      if (!objective_resource->can_team_capture(idx, localplayer->get_team())) continue;
      int owner = objective_resource->get_owning_team(idx);
      if (owner == (int)localplayer->get_team()) continue; // skip if captured
      Vec3 pos = objective_resource->get_origin(idx);
      float d2 = distance_squared_2d(me, pos);
      if (d2 < best_d2) {
        best_d2 = d2;
        target_location = pos;
      }
    }
    return target_location;
  }


  // capture / objective
  static JobResult capture(Player* localplayer) {
    JobResult r{};
    if (!config.navbot.do_objective) return r;
    Vec3 target_location{};
  switch (determine_game_mode(engine->get_level_name())) {
      case GameMode::CTF:  target_location = ctf_target_location(localplayer);  break;
      case GameMode::KOTH: target_location = koth_target_location(localplayer); break;
      default: break;
    }
    if (target_location.x != 0 || target_location.y != 0 || target_location.z != 0) {
      r.kind = Kind::Capture;
      r.has_target = true;
      r.target = target_location;
    }
    return r;
  }

  // stalkenemies / snipe enemies
  static JobResult snipe_enemies(Player* localplayer) {
    JobResult r{};
    if (!config.navbot.snipe) return r;

    const auto& players = entity_cache[class_id::PLAYER];
    float best_d2 = std::numeric_limits<float>::max();
    Player* best = nullptr;
    Vec3 my_pos = localplayer->get_origin();
    for (Entity* e : players) {
      if (!e) continue;
      Player* p = static_cast<Player*>(e);
      if (p == localplayer) continue;
      if (p->get_team() == localplayer->get_team()) continue;
      if (p->get_lifestate() != 1) continue;
      float d2 = distance_squared_2d(my_pos, p->get_origin());
      if (d2 < best_d2) { best_d2 = d2; best = p; }
    }
    if (best) {
      const float radius = 1500.0f;
      float ct = global_vars ? global_vars->curtime : 0.0f;
      constexpr float step_time = 1.0f;
      constexpr float step_angle = 0.7f;
      int step = (int)std::floor(ct / step_time);
      float angle = step * step_angle;
      Vec3 enemy = best->get_origin();
      Vec3 offset{ std::cos(angle) * radius, std::sin(angle) * radius, 0.0f };
      r.kind = Kind::Snipe;
      r.has_target = true;
      r.target = Vec3{ enemy.x + offset.x, enemy.y + offset.y, enemy.z };
    }
    return r;
  }

  // roam / roaming
  static JobResult roam(Player* localplayer, Area* from_area) {
    JobResult r{};
    if (!config.navbot.roaming) return r;

    Vec3 loc = localplayer->get_origin();
    Area* start_area = from_area ? from_area : mesh.find_nearest_area_2d(loc);
    if (start_area != nullptr) {
      int far_id = mesh.pick_far_goal_from_here(start_area, loc.x, loc.y, loc.z);
      Area* to_area = mesh.id_to_area(far_id);
      if (to_area != nullptr) {
        r.kind = Kind::Roam;
        r.has_target = true;
        r.target = to_area->center();
      }
    }
    return r;
  }

  // jobmanager capture = 1, snipe = 2, roam = 3
  static JobResult pick(Player* localplayer, Area* from_area) {
    // choose by ascending priority (1 = highest)
    JobResult cap = capture(localplayer);
    JobResult snp = snipe_enemies(localplayer);
    JobResult rom = roam(localplayer, from_area);

    int pr_cap = config.navbot.jobs.objective_priority;
    int pr_snp = config.navbot.jobs.snipe_priority;
    int pr_rom = config.navbot.jobs.roam_priority;

    const int INF = 1 << 30;
    int best_p = INF;
    int best_tie = INF; // tie-breaker: Capture(0) > Snipe(1) > Roam(2)
    JobResult best{};

    auto consider = [&](const JobResult& r, int p, int tie) {
      if (!r.has_target) return;
      if (p < best_p || (p == best_p && tie < best_tie)) {
        best = r;
        best_p = p;
        best_tie = tie;
      }
    };

    consider(cap, pr_cap, 0);
    consider(snp, pr_snp, 1);
    consider(rom, pr_rom, 2);

    if (best_p != INF) return best;
    return JobResult{}; // no valid job targets
  }
}

void navbot(user_cmd* user_cmd, Vec3 original_view_angles) {
  if (config.navbot.master == false) {
  PathState::Clear();
    return;
  }
  
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) {
  PathState::Clear();
    return;
  }
  
  if (localplayer->get_lifestate() != 1) {
  PathState::Clear();
    return;
  }
  
  Vec3 location = localplayer->get_origin();
  Vec3 target_location{};

  Area* from_area = mesh.best_area_from_xyz(location);

  jobs::JobResult job = jobs::pick(localplayer, from_area);

  static jobs::Kind last_job_kind = jobs::Kind::NoJob;
  if (job.kind != last_job_kind) {
    if (config.debug.debug_draw_navbot_path) {
      auto kind_str = [](jobs::Kind k) {
        switch (k) {
          case jobs::Kind::Capture: return "Capture";
          case jobs::Kind::Snipe:   return "Snipe";
          case jobs::Kind::Roam:    return "Roam";
          default:                  return "NoJob";
        }
      };
      print("[navbot] job -> %s%s\n", kind_str(job.kind), job.has_target ? "" : " (no target)");
    }
    last_job_kind = job.kind;
  }

  static uint32_t active_roam_goal_id = 0;
  static float last_snipe_retarget_time = 0.0f;
  float cur_time_sec = PathState::CurTimeSeconds();

  // shizophrenia fix
  if (job.kind == jobs::Kind::Roam) {
    if (path.goal_id != 0 && active_roam_goal_id == path.goal_id) {
      job.has_target = false;
    }
  } else {
    active_roam_goal_id = 0;
  }

  if (job.kind == jobs::Kind::Snipe) {
  if (path.goal_id != 0 && (cur_time_sec - last_snipe_retarget_time) < 0.75f) {
      job.has_target = false;
    }
  }

  if (job.has_target) {
    target_location = job.target;
  }

  Area* new_target_area = nullptr;
  if (job.has_target) {
    new_target_area = resolve_target_area_for(target_location);
  }
  
  if (from_area != nullptr && new_target_area != nullptr && (path.goal_id == 0 || path.goal_id != new_target_area->id)) {  
    if (set_path_from_to(from_area, new_target_area)) {
      if (job.kind == jobs::Kind::Roam) {
        active_roam_goal_id = new_target_area->id;
      }
      if (job.kind == jobs::Kind::Snipe) {
  last_snipe_retarget_time = cur_time_sec;
      }
    }
  }
  
  // Follow path to target location
  if (path.goal_id > 0 && !path.path_ids.empty()) {
    Area* current_area = mesh.best_area_from_xyz(location);
    // trck visited areas to diversify roaming goals
    if (current_area && current_area->id != path.last_area_id) {
      path.visited_add(current_area->id);
      path.last_area_id = current_area->id;
    }
    
    // stuck detection that works.
    // todo maybe make threshold configurable in menu?
    static uint32_t stuck_on_crumb_id = 0;
    static float stuck_on_crumb_time = 0.0f;
    static float last_crumb_check_time = 0.0f;
    const float crumb_stuck_threshold = 1.2f; // 1.2 seconds on same crumb = stuck
    
  float ct_crumb = PathState::CurTimeSeconds();
    if (ct_crumb - last_crumb_check_time > 0.1f) { // 100ms
      uint32_t current_crumb_id = path.next_index < path.path_ids.size() ? path.path_ids[path.next_index] : 0;
      if (current_crumb_id != 0) {
        if (stuck_on_crumb_id == current_crumb_id) {
          // still stuck on same crumb
          if (stuck_on_crumb_time == 0.0f) {
            stuck_on_crumb_time = ct_crumb;
          } else if (ct_crumb - stuck_on_crumb_time > crumb_stuck_threshold) {
            // try to skip crumb if it doesnt skip as it should for some reason
            int skips = 0;
            int max_skip = std::min(3, (int)path.path_ids.size() - (int)path.next_index - 1);
            for (int i = 0; i < max_skip; i++) {
              if (path.next_index + 1 < path.path_ids.size()) {
                path.next_index++;
                skips++;
              }
            }
            if (skips > 0) {
              stuck_on_crumb_id = 0;
              stuck_on_crumb_time = 0.0f;
            } else {
              // skipping wont work, repath.
              Area* from = current_area ? current_area : mesh.best_area_from_xyz(location);
              Area* goal_area = mesh.id_to_area(path.goal_id);
              if (set_path_from_to(from, goal_area)) {
                stuck_on_crumb_id = 0;
                stuck_on_crumb_time = 0.0f;
              }
            }
          }
        } else {
          stuck_on_crumb_id = current_crumb_id;
          stuck_on_crumb_time = 0.0f;
        }
      }
      last_crumb_check_time = ct_crumb;
    }
    
    Area* next_area = nullptr;
    if (path.next_index < path.path_ids.size()) {
      next_area = mesh.id_to_area(path.path_ids[path.next_index]);
      
      if (next_area && current_area) {
        bool is_connected = areas_connected(current_area, next_area);
        
        if (!is_connected) {
          bool found_connected = false;
          
          for (int skip = 1; skip <= 4 && (path.next_index + skip) < path.path_ids.size(); skip++) {
            Area* skip_area = mesh.id_to_area(path.path_ids[path.next_index + skip]);
            if (skip_area) {
              if (areas_connected(current_area, skip_area)) {
                path.next_index += skip;
                next_area = skip_area;
                found_connected = true;
                break;
              }
            }
          }
          
          if (!found_connected) {
            Area* goal_area = mesh.id_to_area(path.goal_id);
            if (set_path_from_to(current_area, goal_area)) {
              if (path.next_index < path.path_ids.size()) {
                next_area = mesh.id_to_area(path.path_ids[path.next_index]);
              } else {
                next_area = nullptr;
              }
            } else {
              next_area = nullptr;
            }
          }
        }
      }
    }
    
  FinishPathIfOnGoal(current_area);

    advance_if_on_next_area(current_area, next_area);

    // thingy that fixes navbot trying to blindly go to crumb after getting airblasted etc off the track.
    if (path.goal_id != 0 && !path.path_ids.empty() && current_area != nullptr && path.next_index < path.path_ids.size()) {
      size_t found_idx = path.path_ids.size();
      for (size_t i = path.next_index; i < path.path_ids.size(); ++i) {
        if (path.path_ids[i] == current_area->id) { found_idx = i; break; }
      }
      if (found_idx < path.path_ids.size()) {
        path.next_index = (unsigned int)(found_idx + 1);
        if (path.next_index >= path.path_ids.size()) {
          PathState::ResetCurrent();
        } else {
          next_area = mesh.id_to_area(path.path_ids[path.next_index]);
        }
      } else {
        int max_skips = 3;
        while (max_skips-- > 0 && path.next_index + 1 < path.path_ids.size()) {
          Area* na = mesh.id_to_area(path.path_ids[path.next_index]);
          Area* nb = mesh.id_to_area(path.path_ids[path.next_index + 1]);
          if (!na || !nb) break;
          float d_a = distance_2d(location, na->center());
          float d_b = distance_2d(location, nb->center());
          if (d_b + 24.0f < d_a) {
            path.next_index++;
            next_area = nb;
            continue;
          }
          break;
        }
      }
    }

    if (config.navbot.walk == true && next_area != nullptr) {
      Vec3 area_center = next_area->center();
      float x_diff = area_center.x - location.x;
      float y_diff = area_center.y - location.y;
      float dist_squared = x_diff*x_diff + y_diff*y_diff;

      float desired_yaw = std::atan2(y_diff, x_diff) * radpi;
      float current_yaw = original_view_angles.y;
      float delta_yaw   = azimuth_to_signed(desired_yaw - current_yaw);

      float dist = sqrt(dist_squared);
      
      float step = dist > 80.0f ? 80.0f : (dist > 40.0f ? 40.0f : 20.0f);
      float dirx = std::cos(std::atan2(y_diff, x_diff));
      float diry = std::sin(std::atan2(y_diff, x_diff));
      Vec3 ahead = Vec3{ location.x + dirx * step, location.y + diry * step, location.z };
      
      float speed = 350.0f;
      float fwd = std::cos(delta_yaw * pideg) * speed;
      float side = -std::sin(delta_yaw * pideg) * speed;
      
      user_cmd->forwardmove = clampf(fwd, -450.0f, 450.0f);
      user_cmd->sidemove = clampf(side, -450.0f, 450.0f);

      // antistuck and movement helpers
      static float last_progress_time_sd = 0.0f;     // time of last meaningful progress
      static float last_repath_time_sd   = 0.0f;     // cooldown for repathing
      static float phase_start_time_sd   = 0.0f;     // when current phase started
      static float last_yaw_osc_switch   = 0.0f;     // small wiggle timer
      static float last_forced_hop_time  = 0.0f;     // cooldown for hop in wiggle
      static float last_pos_time         = 0.0f;     // last time we sampled raw position
      static Vec3  last_pos              = {};       // raw position sampling
      static int   stuck_phase           = 0;        // 0=normal,1=wiggle,2=backoff,3=repath
      static int   wiggle_dir            = 1;        // +/-1 toggle for strafing
      static float last_offpath_repath_time = 0.0f;  // cooldown for immediate off-path repath
      static bool  pending_progress_reset   = false; // delay-init progress baselines after reset
      static bool  pending_jump_state_reset = false;
      static uint32_t last_goal_id = 0;
      static int last_life_state = 0;
      int life_state_now = localplayer->get_lifestate();
      if (path.goal_id != last_goal_id || life_state_now != last_life_state) {
        last_goal_id = path.goal_id;
        last_life_state = life_state_now;
        last_progress_time_sd = 0.0f;
        last_repath_time_sd   = 0.0f;
        phase_start_time_sd   = 0.0f;
        last_yaw_osc_switch   = 0.0f;
        last_forced_hop_time  = 0.0f;
        last_pos_time         = 0.0f;
        last_pos              = {};
        stuck_phase           = 0;
        wiggle_dir            = 1;
        pending_jump_state_reset = true;
        pending_progress_reset   = true;
      }

  float ct_sd = PathState::CurTimeSeconds();

      // we did some progress, maybe not stuck?
      float dist_to_next_sd = std::sqrt(dist_squared);
      static float last_dist_to_node = 1e9f;
      if (pending_progress_reset) {
        last_dist_to_node = dist_to_next_sd + 8.0f;
        last_progress_time_sd = ct_sd;
        pending_progress_reset = false;
      }
      bool made_progress = (dist_to_next_sd + 4.0f) < last_dist_to_node;
      if (made_progress) {
        last_progress_time_sd = ct_sd;
        last_dist_to_node = dist_to_next_sd;
        stuck_phase = 0;
        phase_start_time_sd = ct_sd;
        wiggle_dir = 1;
      } else {
        last_dist_to_node = std::min(last_dist_to_node + 1.0f, dist_to_next_sd);
      }

      if (ct_sd - last_pos_time > 0.20f) { 
        float dpos2 = distance_squared_2d(location, last_pos);
        if (dpos2 > 16.0f) {
          last_progress_time_sd = ct_sd;
        }
        last_pos = location;
        last_pos_time = ct_sd;
      }

      float since_progress = ct_sd - last_progress_time_sd;

      bool moving_to_goal = (std::fabs(user_cmd->forwardmove) > 10.0f || std::fabs(user_cmd->sidemove) > 10.0f) && (path.goal_id != 0) && (dist_to_next_sd > 48.0f);

      const float t_wiggle = 0.80f; 
      const float t_back   = 1.00f;
      const float t_repath = 1.50f;

      if (moving_to_goal) {
        if (since_progress > t_repath) {
          stuck_phase = 3;
        } else if (since_progress > t_back) {
          stuck_phase = std::max(stuck_phase, 2);
        } else if (since_progress > t_wiggle) {
          stuck_phase = std::max(stuck_phase, 1);
        }
      } else {
        if (stuck_phase > 0 && since_progress < 0.5f) {
          stuck_phase = 0;
        }
      }
      // another offpath detection (yeah this one is also important)
      if (path.goal_id != 0 && next_area) {
        float dz_to_next = std::fabs(next_area->center().z - location.z);
        const float offpath_dz = kJumpHeight * 1.5f + kZSlop;
        if (dz_to_next > offpath_dz && (ct_sd - last_offpath_repath_time) > 0.75f) {
          Area* from = current_area ? current_area : mesh.best_area_from_xyz(location);
          Area* goal_area = mesh.id_to_area(path.goal_id);
          if (set_path_from_to(from, goal_area)) {
            last_offpath_repath_time = ct_sd;
            pending_progress_reset = true;
            stuck_phase = 1;
            phase_start_time_sd = ct_sd;
          }
        }
      }

      auto clamp_move = [](float v) { return clampf(v, -450.0f, 450.0f); };

      if (stuck_phase >= 1) {
        float osc_period = 0.20f;
        if (ct_sd - last_yaw_osc_switch > osc_period) {
          wiggle_dir = -wiggle_dir;
          last_yaw_osc_switch = ct_sd;
        }
        float extra_side = 160.0f * (float)wiggle_dir;
        user_cmd->sidemove = clamp_move(user_cmd->sidemove + extra_side);

        if (config.navbot.look_at_path && !(user_cmd->buttons & IN_ATTACK)) {
          float yaw_scrub = 6.0f * (float)wiggle_dir; // degrees
          user_cmd->view_angles.y = azimuth_to_signed(user_cmd->view_angles.y + yaw_scrub);
        }
      }

      static float last_unstuck_jump_time = 0.0f;
      static bool  cj_active = false;     // currently in a crouchjump sequence
      static int   cj_ticks  = 0;         // ticks since jump started
      static bool  cj_left_ground = false;// have we left the ground since starting?
      static float last_jump_time = 0.0f; // cooldown for any jump

      if (stuck_phase >= 2) {
        float back_strength = 220.0f;
        user_cmd->forwardmove = clamp_move(user_cmd->forwardmove - back_strength);

        float pulse_cd = 0.45f;
        if (!cj_active && localplayer->get_ground_entity() && (ct_sd - last_unstuck_jump_time) > pulse_cd && (ct_sd - last_jump_time) >= 0.2f) {
          // start crouchjump sequence (jump this tick, then hold duck until we land)
          cj_active = true;
          cj_ticks = 0;
          cj_left_ground = false;
          last_unstuck_jump_time = ct_sd;
          last_jump_time = ct_sd;
        }
      }

      if (stuck_phase >= 3) {
        if (ct_sd - last_repath_time_sd > 1.5f) {
          Area* from = current_area ? current_area : mesh.best_area_from_xyz(location);
          Area* goal_area = mesh.id_to_area(path.goal_id);
          set_path_from_to(from, goal_area);
          last_repath_time_sd = ct_sd;
          stuck_phase = 1;
          phase_start_time_sd = ct_sd;
        }
      }

  // normal crouchjump handling
  // (cj_* statics declared above)

      if (pending_jump_state_reset) {
        cj_active = false;
        cj_ticks = 0;
        cj_left_ground = false;
        last_jump_time = 0.0f;
        pending_jump_state_reset = false;
      }

      //cant jump if revved/scoped or in water.
      bool can_jump = true;
      if (localplayer->is_scoped() && !(localplayer->get_flags() & FL_INWATER)) {
        can_jump = false;
      }
      if (localplayer->get_tf_class() == CLASS_HEAVYWEAPONS && (user_cmd->buttons & IN_ATTACK2)) {
        can_jump = false;
      }

      bool prevent_jump = false;
      if (path.next_index + 1 < path.path_ids.size()) {
        Area* next2 = mesh.id_to_area(path.path_ids[path.next_index + 1]);
        if (next2 != nullptr) {
          float dz_drop = next_area->center().z - next2->center().z;
          if (dz_drop < 0.0f && dz_drop <= -kJumpHeight) {
            prevent_jump = true; // big drop ahead, don't jump on the edge
          }
        }
      }

      if (next_area && current_area) {
        Vec3 approach_pt = current_area->closest_point_to_target(next_area->center());
        float d_edge = distance_2d(location, approach_pt);
        const float edge_trigger_dist = kHullWidth * 0.75f + 8.0f;
        float curr_z_samp = current_area->sample_z_at_xy(approach_pt.x, approach_pt.y);
        float next_z_samp = next_area->sample_z_at_xy(approach_pt.x, approach_pt.y);
        float step_height = next_z_samp - curr_z_samp;
        bool need_step_jump = (step_height > (kStepHeight + 1.2f) && step_height <= (kJumpHeight + kZSlop));
        bool near_edge = d_edge <= edge_trigger_dist;

        if (!prevent_jump && can_jump && need_step_jump && near_edge && !cj_active && localplayer->get_ground_entity() && (ct_sd - last_jump_time) >= 0.2f) {
          cj_active = true;
          cj_ticks = 0;
          cj_left_ground = false;
          last_jump_time = ct_sd;
        }
      }

      if (cj_active) {
        if (cj_ticks == 0) {
          user_cmd->buttons |= IN_JUMP;
        }
        user_cmd->buttons |= IN_DUCK;
        cj_ticks++;
        if (!cj_left_ground && !localplayer->get_ground_entity()) {
          cj_left_ground = true;
        }
        if ((cj_left_ground && localplayer->get_ground_entity() && cj_ticks > 3) || cj_ticks > 20) {
          cj_active = false;
        }
      }
      if (config.navbot.look_at_path == true && !(user_cmd->buttons & IN_ATTACK)) {
        float linear_interp = config.navbot.look_smoothness;
        if (config.navbot.look_smoothness <= 0) {
          linear_interp = 1;
        }

        float new_yaw = azimuth_to_signed((delta_yaw / linear_interp) + current_yaw);
        user_cmd->view_angles.y = new_yaw;
        user_cmd->view_angles.x = 0.0;
      }
      
    }
  }

}

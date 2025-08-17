#include "../interfaces/client.hpp"
#include "../interfaces/engine.hpp"
#include "../interfaces/entity_list.hpp"
#include "../interfaces/convar_system.hpp"

#include "../gui/config.hpp"

#include "../classes/player.hpp"
#include "../print.hpp"

#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>

#include "../hacks/navmesh/navengine.hpp"
#include "../hacks/navmesh/navparser.hpp"
#include "../hacks/navmesh/pathfinder.hpp"
#include "../interfaces/engine_trace.hpp"
#include "../hacks/navmesh/navbot/nbcore.hpp"
static const float NAVBOT_HULL_WIDTH = 49.0f;
static const float NAVBOT_HULL_HALF_WIDTH = NAVBOT_HULL_WIDTH * 0.5f;
static const float NAVBOT_HULL_HEIGHT = 83.0f;


#include "../hacks/aimbot/aimbot.cpp"
 
bool (*client_mode_create_move_original)(void*, float, user_cmd*);

bool client_mode_create_move_hook(void* me, float sample_time, user_cmd* user_cmd) {

  bool rc = client_mode_create_move_original(me, sample_time, user_cmd);
  
  if (!user_cmd || !user_cmd->command_number) {
    return rc;
  }

  if (!engine->is_in_game()) {
    return rc;
  }

  Player* localplayer = entity_list->player_from_index(engine->get_localplayer_index());

  if (localplayer == nullptr) {
    print("localplayer is NULL\n");
    return rc;
  }
  
  static bool s_nav_move_set = false;
  static Vec3 s_nav_orig_view = {};
  static float s_nav_orig_forward = 0.0f;
  static float s_nav_orig_side = 0.0f;
  
  if (user_cmd->tick_count > 1) {    
    
    aimbot(user_cmd);
      
    static bool bStaticJump = false, bStaticGrounded = false, bLastAttempted = false;
    const bool bLastJump = bStaticJump;
    const bool bLastGrounded = bStaticGrounded;

    bStaticJump = user_cmd->buttons & IN_JUMP;
    const bool bCurJump = bStaticJump;

    bStaticGrounded = localplayer->get_ground_entity();
    const bool bCurGrounded = bStaticGrounded;

    if (bCurJump && bLastJump && (bCurGrounded ? !localplayer->is_ducking() : true)) {
      if (!(bCurGrounded && !bLastGrounded))
	user_cmd->buttons &= ~IN_JUMP;
      
      if (!(user_cmd->buttons & IN_JUMP) && bCurGrounded && !bLastAttempted)
	user_cmd->buttons |= IN_JUMP;
    }    

    
    //no push
    static Convar* nopush = convar_system->find_var("tf_avoidteammates_pushaway");
    if (nopush != nullptr && config.misc.no_push == true) {
      if (nopush->get_int() != 0) {
        nopush->set_int(0);
      }
    } else if (nopush != nullptr && config.misc.no_push == false) {
      if (nopush->get_int() != 1) {
        nopush->set_int(1);
      }
    }

    /*
    //bhop
    bool on_ground = (localplayer->get_ent_flags() & FL_ONGROUND);

    if(user_cmd->buttons & IN_JUMP && config.misc.bhop == true) {

      if(!was_jumping && !on_ground) {
    	user_cmd->buttons &= ~IN_JUMP;
      } else if(was_jumping) {
    	was_jumping = false;
      }
      
    } else if(!was_jumping) {
      was_jumping = true;
    }
    */
    // Navmesh integration
    // Only run nav logic when local player is alive
    if (config.nav.master && config.nav.engine_enabled && localplayer->get_lifestate() == 1) {
      // Reset nav subsystems on map change (client-side LevelInit equivalent)
      static std::string s_last_map;
      const char* lvl_path = engine ? engine->get_level_name() : nullptr;
      std::string cur_map = nav::ExtractMapNameFromPath(lvl_path);
      if (!cur_map.empty() && cur_map != s_last_map) {
        nav::Visualizer_ClearPath();
        nav::path::Reset();
        nav::navbot::Reset();
        s_last_map = cur_map;
      }
      if (nav::EnsureLoadedForCurrentLevel() && nav::IsLoaded()) {
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

            auto is_passable_segment = [&](const Vec3& from, const Vec3& to, float lift_z) -> bool {
              if (!engine_trace) return true; // fail-open
              Vec3 mins = Vec3{-NAVBOT_HULL_HALF_WIDTH, -NAVBOT_HULL_HALF_WIDTH, 0.0f};
              Vec3 maxs = Vec3{ NAVBOT_HULL_HALF_WIDTH,  NAVBOT_HULL_HALF_WIDTH, NAVBOT_HULL_HEIGHT};
              Vec3 f = from; Vec3 t = to; f.z += lift_z; t.z += lift_z;
              ray_t ray = engine_trace->init_ray((Vec3*)&f, (Vec3*)&t, &mins, &maxs);
              trace_filter filter; engine_trace->init_trace_filter(&filter, (void*)localplayer);
              trace_t tr{};
              engine_trace->trace_ray(&ray, MASK_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MOVEABLE | CONTENTS_GRATE, &filter, &tr);
              return (!tr.all_solid && !tr.start_solid && tr.fraction > 0.98f);
            };

            float dist = std::sqrt(dist2);
            float step = dist > 120.0f ? 120.0f : (dist > 60.0f ? 60.0f : 40.0f);
            float dirx = std::cos(std::atan2(dy, dx));
            float diry = std::sin(std::atan2(dy, dx));
            Vec3 ahead = Vec3{ me.x + dirx * step, me.y + diry * step, me.z };
            const float slope_lift = 18.0f;
            bool forward_clear = is_passable_segment(me, ahead, slope_lift) || is_passable_segment(me, ahead, 0.0f);

            float speed = forward_clear ? 420.0f : 220.0f;
            float fwd = std::cos(rad) * speed;
            float side = -std::sin(rad) * speed;

            if (!forward_clear) {
              float leftx = -diry, lefty = dirx;
              float offset = NAVBOT_HULL_HALF_WIDTH * 1.2f;
              Vec3 ahead_left { me.x + dirx * step + leftx * offset, me.y + diry * step + lefty * offset, me.z };
              Vec3 ahead_right{ me.x + dirx * step - leftx * offset, me.y + diry * step - lefty * offset, me.z };
              bool left_clear = is_passable_segment(me, ahead_left, slope_lift) || is_passable_segment(me, ahead_left, 0.0f);
              bool right_clear = is_passable_segment(me, ahead_right, slope_lift) || is_passable_segment(me, ahead_right, 0.0f);
              if (left_clear != right_clear) {
                side += (left_clear ? +220.0f : -220.0f);
              } else if (!left_clear && !right_clear) {
                fwd *= 0.35f;
              }
            }

            user_cmd->forwardmove = clampf(fwd, -450.0f, 450.0f);
            user_cmd->sidemove = clampf(side, -450.0f, 450.0f);

            if (ro.perform_crouch_jump) {
              user_cmd->buttons |= IN_DUCK;
              if (localplayer->get_ground_entity()) {
                user_cmd->buttons |= IN_JUMP;
              }
            }

            s_nav_move_set = true;
            s_nav_orig_view = user_cmd->view_angles;
            s_nav_orig_forward = user_cmd->forwardmove;
            s_nav_orig_side = user_cmd->sidemove;
          }
        }
      }
    } else {
      nav::Visualizer_ClearPath();
      nav::navbot::Reset();
    }
  } 
  
  if (s_nav_move_set) {
    movement_fix(user_cmd, s_nav_orig_view, s_nav_orig_forward, s_nav_orig_side);
    auto norm_angle = [](float a) {
      a = std::fmod(a, 360.0f);
      if (a > 180.0f) a -= 360.0f;
      if (a <= -180.0f) a += 360.0f;
      return a;
    };
    bool bOrigPitchFlipped = std::fabs(norm_angle(s_nav_orig_view.x)) > 90.0f;
    bool bFinalPitchFlipped = std::fabs(norm_angle(user_cmd->view_angles.x)) > 90.0f;
    if (bOrigPitchFlipped != bFinalPitchFlipped) {
      user_cmd->forwardmove *= -1.0f;
    }
    auto clampf2 = [](float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); };
    user_cmd->forwardmove = clampf2(user_cmd->forwardmove, -450.0f, 450.0f);
    user_cmd->sidemove = clampf2(user_cmd->sidemove, -450.0f, 450.0f);
    s_nav_move_set = false;
  }
  
  if (config.aimbot.silent == true)
    return false;
  else
    return rc;
}

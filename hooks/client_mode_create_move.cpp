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
  nav::CreateMoveResult navRes{};
  
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
    navRes = nav::OnCreateMove(localplayer, user_cmd);
  } 
  
  if (navRes.move_set) {
    float nav_forward = user_cmd->forwardmove;
    float nav_side = user_cmd->sidemove;
    movement_fix(user_cmd, navRes.orig_view, nav_forward, nav_side);
    auto norm_angle = [](float a) {
      a = std::fmod(a, 360.0f);
      if (a > 180.0f) a -= 360.0f;
      if (a <= -180.0f) a += 360.0f;
      return a;
    };
    bool bOrigPitchFlipped = std::fabs(norm_angle(navRes.orig_view.x)) > 90.0f;
    bool bFinalPitchFlipped = std::fabs(norm_angle(user_cmd->view_angles.x)) > 90.0f;
    if (bOrigPitchFlipped != bFinalPitchFlipped) {
      user_cmd->forwardmove *= -1.0f;
    }
    auto clampf2 = [](float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); };
    user_cmd->forwardmove = clampf2(user_cmd->forwardmove, -450.0f, 450.0f);
    user_cmd->sidemove = clampf2(user_cmd->sidemove, -450.0f, 450.0f);
  }
  
  if (config.aimbot.silent == true) {
    return navRes.look_applied ? true : false;
  } else {
    return rc;
  }
}


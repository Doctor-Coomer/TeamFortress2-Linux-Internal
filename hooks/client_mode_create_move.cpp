#include "../interfaces/client.hpp"
#include "../interfaces/engine.hpp"
#include "../interfaces/entity_list.hpp"
#include "../interfaces/convar_system.hpp"

#include "../classes/player.hpp"

#include "../gui/config.hpp"

#include "../print.hpp"

#include "../hacks/aimbot/aimbot.cpp"
#include "../hacks/bhop/bhop.cpp"
#include "../hacks/autovaccinator/autovaccinator.cpp"
#include "../hacks/autobackstab/autobackstab.cpp"
#include "../hacks/navbot/parse_navmesh.cpp"
#include "../hacks/navbot/navbot.cpp"
#include "../hacks/engine_prediction/engine_prediction.cpp"


bool (*client_mode_create_move_original)(void*, float, user_cmd*);

void movement_fix(user_cmd* user_cmd, Vec3 original_view_angle, float original_forward_move, float original_side_move) {
  float yaw_delta = user_cmd->view_angles.y - original_view_angle.y;
  float original_yaw_correction = 0;
  float current_yaw_correction = 0;

  if (original_view_angle.y < 0.0f) {
    original_yaw_correction = 360.0f + original_view_angle.y;
  } else {
    original_yaw_correction = original_view_angle.y;
  }
    
  if (user_cmd->view_angles.y < 0.0f) {
    current_yaw_correction = 360.0f + user_cmd->view_angles.y;
  } else {
    current_yaw_correction = user_cmd->view_angles.y;
  }

  if (current_yaw_correction < original_yaw_correction) {
    yaw_delta = abs(current_yaw_correction - original_yaw_correction);
  } else {
    yaw_delta = 360.0f - abs(original_yaw_correction - current_yaw_correction);
  }
  yaw_delta = 360.0f - yaw_delta;

  user_cmd->forwardmove = cos((yaw_delta) * (M_PI/180)) * original_forward_move + cos((yaw_delta + 90.f) * (M_PI/180)) * original_side_move;
  user_cmd->sidemove = sin((yaw_delta) * (M_PI/180)) * original_forward_move + sin((yaw_delta + 90.f) * (M_PI/180)) * original_side_move;
}

// Called approx every frame.
// Only valid user commands are sent approx 66 times a second
bool client_mode_create_move_hook(void* me, float sample_time, user_cmd* user_cmd) {
  bool rc = client_mode_create_move_original(me, sample_time, user_cmd);
  
  if (user_cmd == nullptr || user_cmd->command_number == 0 || user_cmd->tick_count <= 0) {
    return rc;
  }

  if (!engine->is_in_game()) {
    return rc;
  }
  
  Player* localplayer = entity_list->get_localplayer();  
  if (localplayer == nullptr) {
    print("localplayer is NULL\n");
    return rc;
  }

  /* Run our hacks bellow here */

  Vec3 original_view_angles = user_cmd->view_angles;
  float original_side_move = user_cmd->sidemove;
  float original_forward_move = user_cmd->forwardmove;
    

  bhop(user_cmd);

  autovaccinator(user_cmd);
  autobackstab(user_cmd);
  
  //start_engine_prediction(user_cmd); {
  aimbot(user_cmd, original_view_angles);
  movement_fix(user_cmd, original_view_angles, original_forward_move, original_side_move);
  //}
  //end_engine_prediction();

  
  //No Push
  static Convar* nopush = convar_system->find_var("tf_avoidteammates_pushaway");
  if (nopush != nullptr && config.misc.movement.no_push == true) {
    if (nopush->get_int() != 0) {
      nopush->set_int(0);
    }
  } else if (nopush != nullptr && config.misc.movement.no_push == false) {
    if (nopush->get_int() != 1) {
      nopush->set_int(1);
    }
  }

  // No Engine Sleep
  static Convar* engine_no_focus_sleep = convar_system->find_var("engine_no_focus_sleep");
  if (engine_no_focus_sleep != nullptr && config.misc.exploits.no_engine_sleep == true) {
    if (engine_no_focus_sleep->get_int() != 0) {
      engine_no_focus_sleep->set_int(0);
    }
  } else if (engine_no_focus_sleep != nullptr && config.misc.exploits.no_engine_sleep == false) {
    if (engine_no_focus_sleep->get_int() == 0) {
      engine_no_focus_sleep->set_int(50);
    }
  }
  
  parse_navmesh(engine->get_level_name());
  navbot(user_cmd, original_view_angles);
  if ((config.navbot.look_at_path == true && config.navbot.walk == true && config.navbot.master == true) && !(user_cmd->buttons & IN_ATTACK))
    return rc;
  
  if (config.aimbot.silent == true)
    return false;
  else
    return rc;
}

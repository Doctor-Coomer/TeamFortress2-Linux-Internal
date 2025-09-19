#include <limits.h>

#include "../../interfaces/global_vars.hpp"
#include "../../interfaces/prediction.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/client.hpp"
#include "../../interfaces/game_movement.hpp"
#include "../../interfaces/move_helper.hpp"

#include "../../classes/player.hpp"

#include "../../random_seed.hpp"
#include "../../md5/md5.hpp"

int get_pred_tickbase(user_cmd* user_cmd, Player* localplayer) {
  static int tick = 0;
  static struct user_cmd* last_cmd = nullptr;

  if (user_cmd != nullptr) {
      if (!last_cmd || last_cmd->has_been_predicted)
	tick = localplayer->get_tickbase();
      else
	++tick;

      last_cmd = user_cmd;
    }

  return tick;
}

MoveData move_data = {};
float old_curtime;
float old_frametime;
int old_tickcount;

void start_engine_prediction(user_cmd* user_cmd) {
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr || localplayer->get_lifestate() != 1)
    return;

  old_curtime = global_vars->curtime;
  old_frametime = global_vars->frametime;
  old_tickcount = global_vars->tickcount;

  const int old_tickbase = localplayer->get_tickbase();
  const bool old_first_time_predicted = prediction->first_time_predicted;
  const bool old_in_prediction = prediction->in_prediction;

  move_helper->set_host(localplayer);
    
  localplayer->set_current_cmd(user_cmd);

  *random_seed = md5_pseudo_random(user_cmd->command_number) & INT_MAX;

  global_vars->curtime = get_pred_tickbase(user_cmd, localplayer) * TICK_INTERVAL;
  global_vars->frametime = (prediction->engine_paused ? 0.0f : TICK_INTERVAL);
  global_vars->tickcount = get_pred_tickbase(user_cmd, localplayer);

  prediction->first_time_predicted = false;
  prediction->in_prediction = true;

  prediction->set_local_view_angles(user_cmd->view_angles);

  prediction->setup_move(localplayer, user_cmd, move_helper, &move_data);
  game_movement->process_movement(localplayer, &move_data);
  prediction->finish_move(localplayer, user_cmd, &move_data);
  
  prediction->run_command(localplayer, user_cmd, move_helper);
    
  move_helper->set_host(nullptr);
  localplayer->set_tickbase(old_tickbase);

  prediction->in_prediction = old_in_prediction;
  prediction->first_time_predicted = old_first_time_predicted;

}

void end_engine_prediction() {
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr || localplayer->get_lifestate() != 1)
    return;

  localplayer->set_current_cmd(nullptr);
  
  global_vars->curtime = old_curtime;
  global_vars->frametime = old_frametime;
  global_vars->tickcount = old_tickcount;

  *random_seed = -1;
}

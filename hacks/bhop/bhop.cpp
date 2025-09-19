#include "../../gui/config.hpp"

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"

#include "../../classes/player.hpp"

void bhop(user_cmd* user_cmd) {
  if (config.misc.movement.bhop == false) return;

  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) return;

  static bool static_jump = false, static_grounded = false, last_attempted = false;
  const bool last_jump = static_jump;
  const bool last_grounded = static_grounded;

  static_jump = user_cmd->buttons & IN_JUMP;
  const bool cur_jump = static_jump;

  static_grounded = localplayer->get_ground_entity();
  const bool cur_grounded = static_grounded;

  if (cur_jump == true && last_jump == true && (cur_grounded ? !localplayer->is_ducking() : true)) {
    if (!(cur_grounded == true && last_grounded == false))
      user_cmd->buttons &= ~IN_JUMP;

    if (!(user_cmd->buttons & IN_JUMP) && cur_grounded == true && last_attempted == false)
      user_cmd->buttons |= IN_JUMP;
  }
  
  last_attempted = user_cmd->buttons & IN_JUMP;

}

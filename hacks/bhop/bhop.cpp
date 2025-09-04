#include "../../gui/config.hpp"

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"

#include "../../classes/player.hpp"

void bhop(user_cmd *user_cmd) {
    if (!config.misc.bhop) return;

    Player *localplayer = entity_list->get_localplayer();
    if (localplayer == nullptr) return;

    /*
    static bool static_jump = false, static_grounded = false, last_attempted = false;
    const bool last_jump = static_jump;
    const bool last_grounded = static_grounded;
  
    static_jump = user_cmd->buttons & IN_JUMP;
    const bool cur_jump = static_jump;
  
    static_grounded = localplayer->get_ground_entity();
    const bool cur_grounded = static_grounded;
  
    if (cur_jump && last_jump && (cur_grounded ? !localplayer->is_ducking() : true)) {
      if (!(cur_grounded && !last_grounded))
        user_cmd->buttons &= ~IN_JUMP;
        
      if (!(user_cmd->buttons & IN_JUMP) && cur_grounded && !last_attempted)
        user_cmd->buttons |= IN_JUMP;
    }    
    */

    static bool was_jumping = false;
    bool on_ground = (localplayer->get_flags() & FL_ONGROUND);

    if (user_cmd->buttons & IN_JUMP) {
        if (!was_jumping && !on_ground) {
            user_cmd->buttons &= ~IN_JUMP;
        } else if (was_jumping) {
            was_jumping = false;
        }
    } else if (!was_jumping) {
        was_jumping = true;
    }
}

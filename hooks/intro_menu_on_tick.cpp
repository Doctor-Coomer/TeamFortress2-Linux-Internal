#include <unistd.h>

#include "../interfaces/global_vars.hpp"
#include "../interfaces/entity_list.hpp"
#include "../interfaces/engine.hpp"

#include "../classes/player.hpp"

#include "../print.hpp"

void (*intro_menu_on_tick_original)(void*) = NULL;

static float last_time2 = 0.0;

void intro_menu_on_tick_hook(void* me) {
  intro_menu_on_tick_original(me);
  return;

  /*
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) {
    intro_menu_on_tick_original(me);
    return;
  }

  
  // Init start time
  if (last_time2 == 0.0) {
    last_time2 = global_vars->curtime;
  }

  if (global_vars->curtime - last_time2 >= 1) {

    if (localplayer->get_tf_class() == tf_class::UNDEFINED) {
      *(int*)((unsigned long)(me) + 876) = 3;
      *(float*)((unsigned long)(me) + 872) = global_vars->curtime - 1;
    }
    
    last_time2 = global_vars->curtime;

    intro_menu_on_tick_original(me);

    if (localplayer->get_team() == tf_team::UNKNOWN) {
      engine->client_cmd_unrestricted("autoteam");
      engine->client_cmd_unrestricted("menuclosed");
    }
    
    engine->client_cmd_unrestricted("join_class sniper");
    return;
  }

  intro_menu_on_tick_original(me);
  */
  
}

#include <unistd.h>

#include "../gui/config.hpp"
#include "../interfaces/engine.hpp"

#include "../print.hpp"

void (*team_menu_show_panel_original)(void*, bool) = NULL;

void team_menu_show_panel_hook(void* me, bool show) {
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) {
    team_menu_show_panel_original(me, show);
    return;
  }

  if (config.misc.automation.auto_class_select == true && localplayer->get_team() == tf_team::UNKNOWN) {
    team_menu_show_panel_original(me, false);    
  } else {
    team_menu_show_panel_original(me, show);
  }  
}

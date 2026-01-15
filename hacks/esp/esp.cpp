#include "../../imgui/imgui.h"

#include "../../entity_cache.hpp"

#include "../../gui/config.hpp"

#include "../../interfaces/debug_overlay.hpp"

#include "../../classes/player.hpp"

void draw_players_imgui() {
  //if (entity_cache[class_id::PLAYER].size() <= 0) return;

  //ImDrawList* draw_list = ImGui::GetForegroundDrawList();
  
  /*
  Player* localplayer = entity_list->get_localplayer();  
  for (unsigned int i = 0; i < entity_cache[class_id::PLAYER].size(); ++i) {
    Player* player = (Player*)entity_cache[class_id::PLAYER].at(i);
    if (player == nullptr) continue;

    if (player == localplayer                                                                                      || // Ignore Local Player
	player->is_dormant()                                                                                       || // Ignore Dormat (TODO: Add fading effect to dormat players)
	player->get_lifestate() != 1                                                                               || // Ignore Dead
	(player->get_team() == localplayer->get_team() && config.esp.player.team == false && !player->is_friend()) || // Ignore Team
	(player->is_friend() && config.esp.player.friends == false && (config.esp.player.team == false && player->get_team() == localplayer->get_team())) // Ignore Friends
	) 
      {
	continue;
      }

    Vec3 location = player->get_origin();
    Vec3 screen;
    if (!overlay->world_to_screen(&location, &screen)) continue;

    draw_list->AddLine(ImVec2(0, 0), ImVec2(screen.x, screen.y), IM_COL32(255, 255, 255, 255), 2);
  }
  */

}

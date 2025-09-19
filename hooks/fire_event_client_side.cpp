#include <unistd.h>
#include <utility>

#include "../interfaces/game_event_manager.hpp"

#include "../classes/player.hpp"

#include "../hacks/navbot/navmesh.hpp"

#include "../print.hpp"

bool (*fire_event_client_side_original)(void*, GameEvent*) = NULL;

bool fire_event_client_side_hook(void* me, GameEvent* event) {

  std::string event_name = std::string(event->get_name());
  
  //print("2: %s\n", event->get_name());

  // Force repath if we respawn
  if (event_name == "localplayer_respawn") {
    path = Path{};
  }
  
  if (event_name == "teamplay_round_win") {
    
  }

  if (event_name == "item_pickup") {
    Player* obtainer = entity_list->get_player_from_id(event->get_int("userid"));
    if (obtainer != nullptr && !obtainer->is_dormant()) {
      const char* item_name = event->get_string("item");
      if (strstr(item_name, "medkit") || strstr(item_name, "ammopack")) {
	pickup_item_cache.push_back(PickupItem{obtainer->get_origin(), global_vars->curtime + 10});
      }
    }
  }
  
  if (event_name == "player_hurt") {
    // Example usage of an event
    /*
    Player* victim = entity_list->get_player_from_id(event->get_int("userid"));
    if (victim != nullptr) { 
      player_info pinfo;
      if (engine->get_player_info(victim->get_index(), &pinfo)) {
	int damage = event->get_int("damageamount");
    
	print("%s got hit with %d damage\n", pinfo.name, damage);
      }
    }
    */
  }
  
  return fire_event_client_side_original(me, event);
}

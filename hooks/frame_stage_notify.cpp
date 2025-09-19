#include "../interfaces/global_vars.hpp"
#include "../interfaces/steam_friends.hpp"

#include "../classes/player.hpp"

#include "../entity_cache.hpp"

#include "../print.hpp"

enum ClientFrameStage {
  FRAME_UNDEFINED = -1,
  FRAME_START,
  FRAME_NET_UPDATE_START,
  FRAME_NET_UPDATE_POSTDATAUPDATE_START,
  FRAME_NET_UPDATE_POSTDATAUPDATE_END,
  FRAME_NET_UPDATE_END,
  FRAME_RENDER_START,
  FRAME_RENDER_END
};

void (*frame_stage_notify_original)(void*, ClientFrameStage);

static float last_time = 0.0;

void frame_stage_notify_hook(void* me, ClientFrameStage current_stage) {
  frame_stage_notify_original(me, current_stage);

  // Init start time
  if (last_time == 0.0) {
    last_time = global_vars->curtime;
  }

  switch (current_stage) {
  case FRAME_NET_UPDATE_START:
    {

      if (global_vars->curtime - last_time >= 5) {
	friend_cache.clear();
      }  

      entity_cache.clear();

      break;
    }

  case FRAME_NET_UPDATE_END:
    {
      
      for (unsigned int i = 1; i <= entity_list->get_max_entities(); ++i) {
	Entity* entity = entity_list->entity_from_index(i);
	if (entity == nullptr) continue;
    
	switch (entity->get_class_id()) {
	case class_id::PLAYER:
	  {
	    entity_cache[class_id::PLAYER].push_back(entity);
	
	    if (global_vars->curtime - last_time >= 5) {
	      player_info pinfo;
	      if (engine->get_player_info(entity->get_index(), &pinfo) && pinfo.friends_id != 0) { 
		friend_cache[entity] = steam_friends->is_friend(pinfo.friends_id);
	      }
	    }
	
	    break;
	  }
      
	case class_id::CAPTURE_FLAG:
	  entity_cache[class_id::CAPTURE_FLAG].push_back(entity); break;

	case class_id::OBJECTIVE_RESOURCE:
	  entity_cache[class_id::OBJECTIVE_RESOURCE].push_back(entity); break;

	case class_id::SNIPER_DOT:
	  entity_cache[class_id::SNIPER_DOT].push_back(entity); break;

	  
	}

      }

      break;
    }
  }
  
  
  if (global_vars->curtime - last_time >= 5) {
    last_time = global_vars->curtime;
  }
      
}

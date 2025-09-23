#include <string>

#include "../../vec.hpp"

#include "../../classes/player.hpp"
#include "../../classes/capture_flag.hpp"
#include "../../classes/team_objective_resource.hpp"

#include "../../entity_cache.hpp"

enum class game_mode {
  UNKNOWN,
  CTF,
  CP,
  PL,
  PLR,
  KOTH
};

enum game_mode determine_game_mode(const char* level_path) {
  if (level_path == nullptr) return game_mode::UNKNOWN;

  std::string game_mode_name = std::string(level_path);
  game_mode_name = game_mode_name.substr(game_mode_name.find_last_of('/')+1);
  game_mode_name.resize(game_mode_name.find('_'));

  if (game_mode_name == "ctf") {
    return game_mode::CTF;
  } else if (game_mode_name == "cp") { // Maybe check the cvar tf_gamemode_cp instead
    return game_mode::CP;
  } else if (game_mode_name == "pl") {
    return game_mode::PL;
  } else if (game_mode_name == "plr") {
    return game_mode::PLR;
  } else if (game_mode_name == "koth") {
    return game_mode::KOTH;
  }

  return game_mode::UNKNOWN;
}

Vec3 ctf_target_location(Player* localplayer) {
  Vec3 target_location;

  // Locate which flag to travel to
  CaptureFlag* enemy_intelligence = nullptr;
  CaptureFlag* team_intelligence = nullptr;

  for (unsigned int i = 0; i < entity_cache[class_id::CAPTURE_FLAG].size(); ++i) {
    Entity* entity = entity_cache[class_id::CAPTURE_FLAG].at(i);
    if (entity == nullptr) continue;

    CaptureFlag* temp_flag = (CaptureFlag*)entity;
      
    // Target Enemy Flag
    if (entity->get_team() != localplayer->get_team())  {
      enemy_intelligence = temp_flag;
      continue;
    }

    // Returning the Flag to our own
    if (entity->get_team() == localplayer->get_team()) {
      team_intelligence = temp_flag;
      continue;
    }
      
  }

  CaptureFlag* target_intelligence = nullptr;
  
  if (enemy_intelligence != nullptr && team_intelligence != nullptr) {

    if (team_intelligence->get_owner_entity() != nullptr || team_intelligence->get_status() == flag_status::DROPPED) { // Go defend our intel if it was stolen
      target_intelligence = team_intelligence;
    } else if (enemy_intelligence->get_owner_entity() != localplayer) { // Target enemy intel if we are not holding it
      target_intelligence = enemy_intelligence;
    } else if (enemy_intelligence->get_owner_entity() == localplayer && team_intelligence->get_status() == flag_status::HOME) { // Return enemy intel
      target_intelligence = team_intelligence;
    } 

    if (target_intelligence != nullptr)
      target_location = target_intelligence->get_origin();
  }

  return target_location;
}

Vec3 koth_target_location(Player* localplayer) {
  Vec3 target_location;
  
  TeamObjectiveResource* objective_resource = nullptr;
  
  for (unsigned int i = 0; i < entity_cache[class_id::OBJECTIVE_RESOURCE].size(); ++i) {
    Entity* entity = entity_cache[class_id::OBJECTIVE_RESOURCE].at(i);
    if (entity == nullptr) continue;

    objective_resource = (TeamObjectiveResource*)entity;
    break;
  }

  /*
  for (unsigned int i = 0; i < objective_resource->get_num_control_points(); ++i) {
    print("\n\n");
    print("Index: %d\n", i);
    print("Owned team: %d\n", objective_resource->get_owning_team(i));
    print("My team: %d\n", localplayer->get_team());
    Vec3 location = objective_resource->get_origin(i);
    print("Location: %f %f %f\n", location.x, location.y, location.z);
    print("Is locked: %s\n", objective_resource->is_locked(i) ? "true" : "false");
    print("Can team cap: %s\n", objective_resource->can_team_capture(i, localplayer->get_team()) ? "true" : "false");
  }
  */
  
  if (objective_resource->is_locked(0) == false && objective_resource->can_team_capture(0, localplayer->get_team()) == true) { // Target the Hill
    target_location = objective_resource->get_origin(0);
  } 
  
  return target_location;
}

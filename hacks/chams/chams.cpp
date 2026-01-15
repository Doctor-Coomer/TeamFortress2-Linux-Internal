#include "chams.hpp"

#include <string_view>

#include "chams_player.cpp"
#include "chams_wearable.cpp"

#include "../../assert.hpp"

#include "../../gui/config.hpp"

#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/material_system.hpp"

#include "../../classes/keyvalues.hpp"
#include "../../classes/player.hpp"

void chams(Entity* entity, void* me, void* state, ModelRenderInfo* pinfo, VMatrix* bone_to_world) {
  if (config.chams.master == false) DME_RETURN;

  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) {
    DME_RETURN;
  }


  if (materials.empty()) {
    initialize_materials();
  }  

  // If we're still empty, something has gone very wrong.
  error_assert(materials.empty(), "Materials list is still empty even after initialization!");

    
  class_id id = entity->get_class_id();
  if (id == class_id::PLAYER) {
    chams_player((Player*)entity, me, state, pinfo, bone_to_world);
  } else if (entity->is_wearable() || (entity->is_base_combat_weapon() && entity->get_owner_entity() != nullptr && !entity->get_pickup_type())) {
    chams_wearable(entity, me, state, pinfo, bone_to_world); // Attachments on the player model like the gun and hats
  } else {
    draw_model_execute_original(me, state, pinfo, bone_to_world);
  }

  model_render->forced_material_override(nullptr, OVERRIDE_NORMAL);
  RGBA_float white = {1,1,1,1};
  render_view->set_color_modulation(&white);
  render_view->set_blend(1);
}

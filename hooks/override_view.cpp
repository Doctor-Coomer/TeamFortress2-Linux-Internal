#include "../vec.hpp"

#include "../interfaces/entity_list.hpp"
#include "../interfaces/convar_system.hpp"

#include "../gui/config.hpp"

#include "../classes/player.hpp"

#include "../hacks/thirdperson/thirdperson.cpp"

void (*override_view_original)(void*, view_setup*);

void override_view_hook(void* me, view_setup* setup) {
  override_view_original(me, setup);

  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) return;
  
  if (config.visuals.removals.zoom == true) {
    setup->fov = config.visuals.override_fov ? config.visuals.custom_fov : localplayer->get_default_fov();
  } else {
    if (config.visuals.override_fov == true && localplayer->is_scoped() == false)
      setup->fov = config.visuals.custom_fov;
  }
  
  static Convar* viewmodel_fov = convar_system->find_var("viewmodel_fov");
  if (viewmodel_fov != nullptr && config.visuals.override_viewmodel_fov == true) {
    viewmodel_fov->set_float(config.visuals.custom_viewmodel_fov);
  }
  
  thirdperson(setup);
}

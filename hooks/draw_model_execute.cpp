#include "../hacks/chams/chams.hpp"

#include <string.h>

#include "../vec.hpp"
#include "../assert.hpp"

#include "../gui/config.hpp"

#include "../interfaces/entity_list.hpp"
#include "../interfaces/material_system.hpp"
#include "../interfaces/model_render.hpp"
#include "../interfaces/render_view.hpp"

#include "../classes/material.hpp"
#include "../classes/player.hpp"

#include "../hacks/chams/chams.cpp"

#include "../print.hpp"

struct model_t {
  void* handle;
  char* name;
  int load_flags;
  int server_count;
  int type;
  int flags;
  Vec3 vec_mins;
  Vec3 vec_maxs;
  float radius;
};


struct ModelRenderInfo {
  Vec3 origin;
  Vec3 angles;
  void* pRenderable;
  const model_t* model;
  const VMatrix* model_to_world;
  const VMatrix* lighting_offset;
  const Vec3* lighting_origin;
  int flags;
  int entity_index;
  int skin;
  int body;
  int hitboxset;
  short instance;
};

void draw_model_execute_hook(void* me, void* state, ModelRenderInfo* pinfo, VMatrix* bone_to_world) {  
  
  Entity* entity = entity_list->entity_from_index(pinfo->entity_index);
  if (entity == nullptr) {
    DME_RETURN;
  }

  chams(entity, me, state, pinfo, bone_to_world);
  
  /*
  
  Material* material = materials.at(0);

  RGBA_float color = {1, 0, 1, 1};
  render_view->set_color_modulation(&color);
  render_view->set_blend(0.5);

  
  model_render->forced_material_override(material);
  material->set_material_flag(MATERIAL_VAR_WIREFRAME, true);

  draw_model_execute_original(me, state, pinfo, bone_to_world);

  model_render->forced_material_override(nullptr);

  RGBA_float white = {1,1,1,1};
  render_view->set_color_modulation(&white);
  render_view->set_blend(1);
  */
}

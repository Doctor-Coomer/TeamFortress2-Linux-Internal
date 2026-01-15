#ifndef CHAMS_HPP
#define CHAMS_HPP

#include <cassert>
#include <vector>
#include <string_view>

#include "../../assert.hpp"

#include "../../interfaces/material_system.hpp"
#include "../../interfaces/model_render.hpp"
#include "../../interfaces/render_view.hpp"

#include "../../classes/material.hpp"
#include "../../classes/material_var.hpp"


struct ModelRenderInfo;
void (*draw_model_execute_original)(void*, void*, ModelRenderInfo*, VMatrix*);

#define DME_RETURN { draw_model_execute_original(me, state, pinfo, bone_to_world); return; }

inline static std::vector<Material*> materials;

static Material* create_material_ex(const std::string_view name, const std::string_view shader, const std::string_view material) {
  KeyValues* values = new KeyValues(shader.data());
  values->load_from_buffer(name.data(), material.data());
  return material_system->create_material(name.data(), values);
}

static void initialize_materials(void) {
  materials.push_back(nullptr);
  
  materials.push_back(create_material_ex("TfFlat",
					 "UnlitGeneric",
					 R"#(
"UnlitGeneric" {
   "$basetexture" "white"
}
)#"));
  
  materials.push_back(create_material_ex("TfShaded",
					 "VertexLitGeneric",
					 R"#(
"VertexLitGeneric" {
   "$basetexture" "white"
}
)#"));

  materials.push_back(create_material_ex("TfFresnel",
					 "VertexLitGeneric",
					 R"#(
"VertexLitGeneric" {
   "$basetexture" "white"
   "$bumpmap" "models/player/shared/shared_normal"
   "$color2" "[0 0 0]"
   "$additive" "1"
   "$phong" "1"
   "$phongfresnelranges" "[0 0.5 1]"
   "$envmap" "skybox/sky_dustbowl_01"
   "$envmapfresnel" "1"
   "$selfillumtint" "[0 0 0]"
   "$envmaptint" "[0 0 0]"
}
)#"));
  
}

static void set_material_information(Material* material, RGBA_float color, bool wireframe = false, bool ignore_z = false, OverrideType override_type = OVERRIDE_NORMAL) {
  error_assert(material == nullptr, "Material is null in set_material_color_flags()!");
  
  render_view->set_color_modulation(&color);
  render_view->set_blend(color.a);
  
  if (material == materials.at(3)) {
    MaterialVar* envmaptint = material->find_var("$envmaptint");
    if (envmaptint != nullptr) {
      envmaptint->set_vec_value(color);
    }
  }
  
  material->set_material_flag(MATERIAL_VAR_IGNOREZ, ignore_z);
  material->set_material_flag(MATERIAL_VAR_WIREFRAME, wireframe);
  model_render->forced_material_override(material, override_type);
}

#endif

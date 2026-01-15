#ifndef MODEL_RENDER_HPP
#define MODEL_RENDER_HPP

#include "../classes/material.hpp"

enum OverrideType {
  OVERRIDE_NORMAL = 0,
  OVERRIDE_BUILD_SHADOWS,
  OVERRIDE_DEPTH_WRITE,
  OVERRIDE_SSAO_DEPTH_WRITE,
};

class ModelRender {
public:
  void forced_material_override(Material* new_material, OverrideType override_type = OVERRIDE_NORMAL) {
    void** vtable = *(void ***)this;

    void (*forced_material_override_fn)(void*, Material*, int) =
      (void (*)(void*, Material*, int))vtable[1];

    forced_material_override_fn(this, new_material, override_type);
  }

  
};

inline static ModelRender* model_render;

#endif

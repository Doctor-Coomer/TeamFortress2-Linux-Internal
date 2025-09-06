#ifndef MODEL_RENDER_HPP
#define MODEL_RENDER_HPP

#include "../classes/material.hpp"

class ModelRender {
public:
    void forced_material_override(Material *new_material, int override_type) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*forced_material_override_fn)(void *, Material *, int) =
                (void (*)(void *, Material *, int)) vtable[1];

        forced_material_override_fn(this, new_material, override_type);
    }
};

inline static ModelRender *model_render;

#endif

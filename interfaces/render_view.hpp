#ifndef RENDER_VIEW_HPP
#define RENDER_VIEW_HPP

#include "../vec.hpp"

#include "../interfaces/client.hpp"
#include "../interfaces/engine.hpp"


enum MaterialVarFlags {
  MATERIAL_VAR_DEBUG                    = (1 << 0),
  MATERIAL_VAR_NO_DEBUG_OVERRIDE        = (1 << 1),
  MATERIAL_VAR_NO_DRAW                  = (1 << 2),
  MATERIAL_VAR_USE_IN_FILLRATE_MODE     = (1 << 3),
  MATERIAL_VAR_VERTEXCOLOR              = (1 << 4),
  MATERIAL_VAR_VERTEXALPHA              = (1 << 5),
  MATERIAL_VAR_SELFILLUM                = (1 << 6),
  MATERIAL_VAR_ADDITIVE                 = (1 << 7),
  MATERIAL_VAR_ALPHATEST                = (1 << 8),
  MATERIAL_VAR_MULTIPASS                = (1 << 9),
  MATERIAL_VAR_ZNEARER                  = (1 << 10),
  MATERIAL_VAR_MODEL                    = (1 << 11),
  MATERIAL_VAR_FLAT                     = (1 << 12),
  MATERIAL_VAR_NOCULL                   = (1 << 13),
  MATERIAL_VAR_NOFOG                    = (1 << 14),
  MATERIAL_VAR_IGNOREZ                  = (1 << 15),
  MATERIAL_VAR_DECAL                    = (1 << 16),
  MATERIAL_VAR_ENVMAPSPHERE             = (1 << 17),
  MATERIAL_VAR_NOALPHAMOD               = (1 << 18),
  MATERIAL_VAR_ENVMAPCAMERASPACE        = (1 << 19),
  MATERIAL_VAR_BASEALPHAENVMAPMASK      = (1 << 20),
  MATERIAL_VAR_TRANSLUCENT              = (1 << 21),
  MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
  MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING  = (1 << 23),
  MATERIAL_VAR_OPAQUETEXTURE            = (1 << 24),
  MATERIAL_VAR_ENVMAPMODE               = (1 << 25),
  MATERIAL_VAR_SUPPRESS_DECALS          = (1 << 26),
  MATERIAL_VAR_HALFLAMBERT              = (1 << 27),
  MATERIAL_VAR_WIREFRAME                = (1 << 28),
  MATERIAL_VAR_ALLOWALPHATOCOVERAGE     = (1 << 29),
  MATERIAL_VAR_IGNORE_ALPHA_MODULATION  = (1 << 30),
};


class RenderView {
public:
  void get_matrices_for_view(const view_setup& view, VMatrix* world_to_screen, VMatrix* view_to_projection, VMatrix* world_to_projection, VMatrix* world_to_pixels) {
    void** vtable = *(void ***)this;

    void (*get_matrices_for_view_fn)(void*, const view_setup&, VMatrix*, VMatrix*, VMatrix*, VMatrix*) =
      (void (*)(void*, const view_setup&, VMatrix*, VMatrix*, VMatrix*, VMatrix*))vtable[50];

    get_matrices_for_view_fn(this, view, world_to_screen, view_to_projection, world_to_projection, world_to_pixels);
  }
  
  bool world_to_screen(Vec3* point, Vec3* screen) {
    view_setup local_view = {};
    client->get_player_view(local_view);

    VMatrix world_to_screen = {};
    VMatrix view_to_pojection = {};
    VMatrix world_to_projection = {};
    VMatrix world_to_pixels = {};
    get_matrices_for_view(local_view, &world_to_screen, &view_to_pojection, &world_to_projection, &world_to_pixels);

    Vec2 screen_size = engine->get_screen_size();
    
    float w = world_to_projection[3][0] * point->x + world_to_projection[3][1] * point->y + world_to_projection[3][2] * point->z + world_to_projection[3][3];

    if (w < 0.1f) return false;

    float vOutTmp[2];
  
    vOutTmp[0] = world_to_projection[0][0] * point->x + world_to_projection[0][1] * point->y + world_to_projection[0][2] * point->z + world_to_projection[0][3];
    vOutTmp[1] = world_to_projection[1][0] * point->x + world_to_projection[1][1] * point->y + world_to_projection[1][2] * point->z + world_to_projection[1][3];
    float invw = 1.0f / w;

    vOutTmp[0] *= invw;
    vOutTmp[1] *= invw;

    float x = screen_size.x / 2.0f;
    float y = screen_size.y / 2.0f;

    x += 0.5f * vOutTmp[0] * screen_size.x + 0.5f;
    y -= 0.5f * vOutTmp[1] * screen_size.y + 0.5f;
  
    screen->x = x * 100.f / 100.f;
    screen->y = y * 100.f / 100.f;
  
    return true;
  }

  void set_color_modulation(const RGBA_float* blend) {
    void** vtable = *(void ***)this;

    void (*set_color_modulation_fn)(void*, const RGBA_float*) = (void (*)(void*, const RGBA_float*))vtable[6];

    set_color_modulation_fn(this, blend);
  }

  void get_color_modulation(RGBA_float* blend) {
    void** vtable = *(void ***)this;

    void (*get_color_modulation_fn)(void*, RGBA_float*) = (void (*)(void*, RGBA_float*))vtable[7];

    get_color_modulation_fn(this, blend);
  }  
  
  void set_blend(float blend) {
    void** vtable = *(void ***)this;

    void (*set_blend_fn)(void*, float) = (void (*)(void*, float))vtable[4];

    set_blend_fn(this, blend);
  }
  
  float get_blend() {
    void** vtable = *(void ***)this;

    float (*get_blend_fn)(void*) = (float (*)(void*))vtable[5];

    return get_blend_fn(this);
  }
  
};

inline static RenderView* render_view;

#endif


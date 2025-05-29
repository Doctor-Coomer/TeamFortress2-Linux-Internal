#ifndef RENDER_VIEW_HPP
#define RENDER_VIEW_HPP

#include "vec.hpp"

#include "client.hpp"
#include "engine.hpp"

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
};

inline static RenderView* render_view;

#endif


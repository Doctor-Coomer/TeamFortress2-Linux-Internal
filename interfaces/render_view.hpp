#ifndef RENDER_VIEW_HPP
#define RENDER_VIEW_HPP

#include "../vec.hpp"

#include "../interfaces/client.hpp"
#include "../interfaces/engine.hpp"

class RenderView {
public:
    void get_matrices_for_view(const view_setup &view, VMatrix *world_to_screen, VMatrix *view_to_projection,
                               VMatrix *world_to_projection, VMatrix *world_to_pixels) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*get_matrices_for_view_fn)(void *, const view_setup &, VMatrix *, VMatrix *, VMatrix *, VMatrix *) =
                (void (*)(void *, const view_setup &, VMatrix *, VMatrix *, VMatrix *, VMatrix *)) vtable[50];

        get_matrices_for_view_fn(this, view, world_to_screen, view_to_projection, world_to_projection, world_to_pixels);
    }

    bool world_to_screen(const Vec3 *point, Vec3 *screen) {
        view_setup local_view = {};
        client->get_player_view(local_view);

        VMatrix world_to_screen = {};
        VMatrix view_to_pojection = {};
        VMatrix world_to_projection = {};
        VMatrix world_to_pixels = {};
        get_matrices_for_view(local_view, &world_to_screen, &view_to_pojection, &world_to_projection, &world_to_pixels);

        const Vec2 screen_size = engine->get_screen_size();

        const float w = world_to_projection[3][0] * point->x + world_to_projection[3][1] * point->y +
                        world_to_projection[3][
                            2] * point->z + world_to_projection[3][3];

        if (w < 0.1f) return false;

        float vOutTmp[2];

        vOutTmp[0] = world_to_projection[0][0] * point->x + world_to_projection[0][1] * point->y + world_to_projection[
                         0][2] * point->z + world_to_projection[0][3];
        vOutTmp[1] = world_to_projection[1][0] * point->x + world_to_projection[1][1] * point->y + world_to_projection[
                         1][2] * point->z + world_to_projection[1][3];
        const float invw = 1.0f / w;

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

    void set_color_modulation(const RGBA_float *blend) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*set_color_modulation_fn)(void *, const RGBA_float *) = (void (*)(void *, const RGBA_float *)) vtable[6];

        set_color_modulation_fn(this, blend);
    }

    void get_color_modulation(RGBA_float *blend) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*get_color_modulation_fn)(void *, RGBA_float *) = (void (*)(void *, RGBA_float *)) vtable[7];

        get_color_modulation_fn(this, blend);
    }

    void set_blend(float blend) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*set_blend_fn)(void *, float) = (void (*)(void *, float)) vtable[4];

        set_blend_fn(this, blend);
    }

    float get_blend() {
        void **vtable = *reinterpret_cast<void ***>(this);

        float (*get_blend_fn)(void *) = (float (*)(void *)) vtable[5];

        return get_blend_fn(this);
    }
};

inline static RenderView *render_view;

#endif

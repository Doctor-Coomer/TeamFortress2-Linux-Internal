#include "../vec.hpp"

#include "../print.hpp"

void (*draw_view_models_original)(void *, view_setup *, bool);

void draw_view_models_hook(void *me, view_setup *setup, bool draw_view_models) {
    draw_view_models_original(me, setup, draw_view_models);
}

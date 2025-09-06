#include "../gui/config.hpp"

#include <string>

void (*paint_traverse_original)(void *, void *, __int8_t, __int8_t) = nullptr;

void *vgui;

const char *get_panel_name(void *panel) {
    void **vtable = *static_cast<void ***>(vgui);

    const char * (*get_panel_name_fn)(void *, void *) = (const char* (*)(void *, void *)) vtable[37];

    return get_panel_name_fn(vgui, panel);
}


void paint_traverse_hook(void *me, void *panel, __int8_t force_repaint, __int8_t allow_force) {
    const std::string panel_name = get_panel_name(panel);

    // skip the original function to hide elements
    if (config.visuals.removals.scope && panel_name == "HudScope") {
        return;
    }

    paint_traverse_original(me, panel, force_repaint, allow_force);

    //print("%s\n", panel_name.c_str());
}

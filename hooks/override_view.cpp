#include "../vec.hpp"

#include "../interfaces/entity_list.hpp"

#include "../gui/config.hpp"

#include "../classes/player.hpp"

#include "../hacks/thirdperson/thirdperson.cpp"

void (*override_view_original)(void *, view_setup *);

void override_view_hook(void *me, view_setup *setup) {
    override_view_original(me, setup);

    Player *localplayer = entity_list->get_localplayer();
    if (localplayer == nullptr) return;

    if (config.visuals.removals.zoom) {
        setup->fov = config.visuals.override_fov ? config.visuals.custom_fov : localplayer->get_default_fov();
    } else {
        if (config.visuals.override_fov && !localplayer->is_scoped())
            setup->fov = config.visuals.custom_fov;
    }

    thirdperson(setup);
}

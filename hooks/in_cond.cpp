#include "../gui/config.hpp"

#include "../classes/player.hpp"

bool in_cond_hook(void *me, int mask) {
    if (mask == TF_COND_ZOOMED && config.visuals.removals.scope) {
        //if they player is scoped
        return false;
    }

    bool re = in_cond_original(me, mask);

    return re;
}

#ifndef DEBUG_OVERLAY_HPP
#define DEBUG_OVERLAY_HPP

#include "../vec.hpp"

class DebugOverlay {
public:
    bool world_to_screen(Vec3 *point, Vec3 *screen) {
        void **vtable = *reinterpret_cast<void ***>(this);

        int (*world_to_screen_fn)(void *, Vec3 *, Vec3 *) = (int (*)(void *, Vec3 *, Vec3 *)) vtable[9];
        int success = world_to_screen_fn(this, point, screen);

        return (success == 0);
    }
};

static inline DebugOverlay *overlay;

#endif

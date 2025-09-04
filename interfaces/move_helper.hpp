#ifndef MOVE_HELPER_HPP
#define MOVE_HELPER_HPP

class Player;

class MoveHelper {
public:
    void set_host(Player *player) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*set_host_fn)(void *, Player *) = (void (*)(void *, Player *)) vtable[12];

        set_host_fn(this, player);
    }
};

inline static MoveHelper *move_helper;

#endif

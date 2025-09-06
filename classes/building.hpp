#ifndef BUILDING_HPP
#define BUILDING_HPP

#include "entity.hpp"

// Maybe make Building a base class for the different buildings
class Building : public Entity {
public:
    int get_health(void) {
        return *reinterpret_cast<int *>(this + 0x1388);
    }

    int get_max_health(void) {
        return *reinterpret_cast<int *>(this + 0x138C);
    }

    bool is_sapped(void) {
        return *reinterpret_cast<bool *>(this + 0x1380);
    }

    bool is_carried(void) {
        return *reinterpret_cast<bool *>(this + 0x1396) || this->is_carried_deploy();
    }

    bool is_carried_deploy(void) {
        return *reinterpret_cast<bool *>(this + 0x1397);
    }

    // Sentry
    bool is_mini_sentry(void) {
        return *reinterpret_cast<bool *>(this + 0x1399);
    }

    int get_building_level(void) {
        return *reinterpret_cast<int *>(this + 0x1334);
    }
};

#endif

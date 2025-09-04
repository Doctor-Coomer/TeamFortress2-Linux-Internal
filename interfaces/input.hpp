#ifndef INPUT_HPP
#define INPUT_HPP

#include "client.hpp"

struct CameraThirdData_t {
    float pitch;
    float yaw;
    float dist;
    float lag;
    Vec3 hull_min;
    Vec3 hull_max;
};

static user_cmd * (*get_user_cmd_original)(void *, int sequence_number);

class Input {
public:
    user_cmd *get_user_cmd(int sequence_number) {
        user_cmd *usercmd = &this->commands[sequence_number % 90];

        return usercmd;
    }

    void to_thirdperson(void) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*to_thirdperson_fn)(void *) = (void (*)(void *)) vtable[32];

        to_thirdperson_fn(this);
    }

    bool is_thirdperson(void) {
        void **vtable = *reinterpret_cast<void ***>(this);

        bool (*is_thirdperson_fn)(void *) = (bool (*)(void *)) vtable[31];

        return is_thirdperson_fn(this);
    }

    void to_firstperson(void) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*to_firstperson_fn)(void *) = (void (*)(void *)) vtable[33];

        to_firstperson_fn(this);
    }

public:
    char pad0[0x100];

    user_cmd *commands;
};

inline static Input *input;

#endif

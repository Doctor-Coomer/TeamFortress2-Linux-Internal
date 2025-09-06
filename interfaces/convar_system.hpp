#ifndef CONVAR_SYSTEM_HPP
#define CONVAR_SYSTEM_HPP

#include "../classes/convar.hpp"

class ConvarSystem {
public:
    Convar *find_var(const char *var_name) {
        void **vtable = *reinterpret_cast<void ***>(this);

        Convar * (*find_var_fn)(void *, const char *) = (Convar* (*)(void *, const char *)) vtable[12];

        return find_var_fn(this, var_name);
    }
};

inline static ConvarSystem *convar_system;

#endif

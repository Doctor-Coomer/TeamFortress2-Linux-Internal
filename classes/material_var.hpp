#ifndef MATERIAL_VAR_HPP
#define MATERIAL_VAR_HPP

#include "../vec.hpp"

class MaterialVar {
public:
  //11
  void set_vec_value(RGBA_float color) {
    void** vtable = *(void***)this;

    void (*set_vec_value_fn)(void*, float, float, float) = (void (*)(void*, float, float, float))vtable[11];

    set_vec_value_fn(this, color.r, color.g, color.b);
  }
};

#endif

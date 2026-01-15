#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "../vec.hpp"

#include "material_var.hpp"

class Material {
public:

  //11
  MaterialVar* find_var(const char* var_name, bool* found = nullptr, bool complain = false) {
    void** vtable = *(void***)this;

    MaterialVar* (*find_var_fn)(void*, const char*, bool*, bool) = (MaterialVar* (*)(void*, const char*, bool*, bool))vtable[11];

    return find_var_fn(this, var_name, found, complain);    
  }
  

  void increment_reference_count(void) {
    void** vtable = *(void***)this;

    void (*increment_reference_count_fn)(void*) = (void (*)(void*))vtable[12];

    increment_reference_count_fn(this);
  }

  void alpha_modulate(float alpha) {
    void** vtable = *(void***)this;

    void (*alpha_modulate_fn)(void*, float) = (void (*)(void*, float))vtable[27];

    alpha_modulate_fn(this, alpha);
  }
  
  void color_modulate(RGBA_float color) {
    void** vtable = *(void***)this;
    
    void (*color_modulate_fn)(void*, float, float, float) = (void (*)(void*, float, float, float))vtable[28];

    color_modulate_fn(this, color.r, color.g, color.b);
  }

  void set_material_flag(int flag, bool on) {
    void** vtable = *(void***)this;

    void (*set_material_flag_fn)(void*, int, bool) = (void (*)(void*, int, bool))vtable[29];
    
    set_material_flag_fn(this, flag, on);
  }

  bool get_material_flag(int flag) {
    void** vtable = *(void***)this;

    bool (*get_material_flag_fn)(void*, int) = (bool (*)(void*, int))vtable[30];

    return get_material_flag_fn(this, flag);
  }
};

#endif

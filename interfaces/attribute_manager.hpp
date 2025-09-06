#ifndef ATTRIBUTE_MANAGER_HPP
#define ATTRIBUTE_MANAGER_HPP

class Entity;

static float (*attribute_hook_value_float_original)(float, const char *, Entity *, void *, bool);

class AttributeManager {
public:
    float attrib_hook_value(float value, const char *attrib_name, Entity *entity) {
        return attribute_hook_value_float_original(value, attrib_name, entity, nullptr, true);
    }
};

inline static AttributeManager *attribute_manager = new AttributeManager();

#endif

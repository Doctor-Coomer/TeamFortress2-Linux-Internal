#ifndef MATERIAL_HPP
#define MATERIAL_HPP

class Material {
public:
    void alpha_modulate(float alpha) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*alpha_modulate_fn)(void *, float) = (void (*)(void *, float)) vtable[27];

        alpha_modulate_fn(this, alpha);
    }

    void color_modulate(float r, float g, float b) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*color_modulate_fn)(void *, float, float, float) = (void (*)(void *, float, float, float)) vtable[28];

        color_modulate_fn(this, r, g, b);
    }

    void set_material_flag(int flag, bool on) {
        void **vtable = *reinterpret_cast<void ***>(this);

        void (*set_material_flag_fn)(void *, int, bool) = (void (*)(void *, int, bool)) vtable[29];

        set_material_flag_fn(this, flag, on);
    }

    bool get_material_flag(int flag) {
        void **vtable = *reinterpret_cast<void ***>(this);

        bool (*get_material_flag_fn)(void *, int) = (bool (*)(void *, int)) vtable[30];

        return get_material_flag_fn(this, flag);
    }
};

struct Material_t {
    Material *m_pMaterial;

    const char *m_sName;
    const char *m_sVMT;
    bool m_bLocked = false;

    bool m_bStored = false;
    void *m_phongtint = nullptr; //IMaterialVar
    void *m_envmaptint = nullptr; //IMaterialVar
    bool m_bInvertCull = false;
};

#endif

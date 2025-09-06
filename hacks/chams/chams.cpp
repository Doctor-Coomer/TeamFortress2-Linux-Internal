#include "../../interfaces/material_system.hpp"
#include "../../classes/material.hpp"

#include "../../interfaces/render_view.hpp"
#include "../../interfaces/model_render.hpp"
#include "../../interfaces/entity_list.hpp"

#include "../../print.hpp"


struct model_t {
    void *handle;
    char *name;
    int load_flags;
    int server_count;
    int type;
    int flags;
    Vec3 vec_mins;
    Vec3 vec_maxs;
    float radius;
};


typedef struct {
    Vec3 origin;
    Vec3 angles;
    void *pRenderable;
    const model_t *model;
    const VMatrix *model_to_world;
    const VMatrix *lighting_offset;
    const Vec3 *lighting_origin;
    int flags;
    int entity_index;
    int skin;
    int body;
    int hitboxset;
    short instance;
} ModelRenderInfo_t;

enum MaterialVarFlags_t {
    MATERIAL_VAR_DEBUG = 1 << 0,
    MATERIAL_VAR_NO_DEBUG_OVERRIDE = 1 << 1,
    MATERIAL_VAR_NO_DRAW = 1 << 2,
    MATERIAL_VAR_USE_IN_FILLRATE_MODE = 1 << 3,
    MATERIAL_VAR_VERTEXCOLOR = 1 << 4,
    MATERIAL_VAR_VERTEXALPHA = 1 << 5,
    MATERIAL_VAR_SELFILLUM = 1 << 6,
    MATERIAL_VAR_ADDITIVE = 1 << 7,
    MATERIAL_VAR_ALPHATEST = 1 << 8,
    MATERIAL_VAR_MULTIPASS = 1 << 9,
    MATERIAL_VAR_ZNEARER = 1 << 10,
    MATERIAL_VAR_MODEL = 1 << 11,
    MATERIAL_VAR_FLAT = 1 << 12,
    MATERIAL_VAR_NOCULL = 1 << 13,
    MATERIAL_VAR_NOFOG = 1 << 14,
    MATERIAL_VAR_IGNOREZ = 1 << 15,
    MATERIAL_VAR_DECAL = 1 << 16,
    MATERIAL_VAR_ENVMAPSPHERE = 1 << 17,
    MATERIAL_VAR_NOALPHAMOD = 1 << 18,
    MATERIAL_VAR_ENVMAPCAMERASPACE = 1 << 19,
    MATERIAL_VAR_BASEALPHAENVMAPMASK = 1 << 20,
    MATERIAL_VAR_TRANSLUCENT = 1 << 21,
    MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = 1 << 22,
    MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = 1 << 23,
    MATERIAL_VAR_OPAQUETEXTURE = 1 << 24,
    MATERIAL_VAR_ENVMAPMODE = 1 << 25,
    MATERIAL_VAR_SUPPRESS_DECALS = 1 << 26,
    MATERIAL_VAR_HALFLAMBERT = 1 << 27,
    MATERIAL_VAR_WIREFRAME = 1 << 28,
    MATERIAL_VAR_ALLOWALPHATOCOVERAGE = 1 << 29,
    MATERIAL_VAR_IGNORE_ALPHA_MODULATION = 1 << 30,
};

enum OverrideType_t {
    OVERRIDE_NORMAL = 0,
    OVERRIDE_BUILD_SHADOWS,
    OVERRIDE_DEPTH_WRITE,
    OVERRIDE_SSAO_DEPTH_WRITE,
};

void (*draw_model_execute_original)(void *, void *, ModelRenderInfo_t *, VMatrix *);

void override_material(const bool ignorez, const bool wireframe, const RGBA_float color, const char *mat_name) {
    Material *material = material_system->find_material(mat_name, "Model textures", true, nullptr);

    if (!material)
        return;

    print("%p\n", material);

    render_view->set_color_modulation(&color);
    render_view->set_blend(color.a);

    material->set_material_flag(MATERIAL_VAR_IGNOREZ, ignorez);
    material->set_material_flag(MATERIAL_VAR_WIREFRAME, wireframe);

    model_render->forced_material_override(material, OVERRIDE_NORMAL);
}

void chams(void *me, void *state, ModelRenderInfo_t *pinfo, VMatrix *bone_to_world) {
    if (pinfo->entity_index < 1 || pinfo->entity_index >= entity_list->get_max_entities()) {
        constexpr RGBA_float color = {1, 0, 1, 1};

        override_material(false, true, color, "dev/glow_color");
        return;
    }
}

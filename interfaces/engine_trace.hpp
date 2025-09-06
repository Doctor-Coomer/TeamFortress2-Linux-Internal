#ifndef ENGINE_TRACE_HPP
#define ENGINE_TRACE_HPP

#include "../vec.hpp"

enum trace_type_t {
    TRACE_EVERYTHING = 0,
    TRACE_WORLD_ONLY,
    TRACE_ENTITIES_ONLY,
    TRACE_EVERYTHING_FILTER_PROPS,
};

struct ray_t {
    Vec3_aligned start; // starting point, centered within the extents
    Vec3_aligned delta; // direction + length of the ray
    Vec3_aligned start_offset; // Add this to m_Start to get the actual ray start
    Vec3_aligned extents; // Describes an axis aligned box extruded along a ray
    bool is_ray; // are the extents zero?
    bool is_swept; // is delta != 0?
};

struct trace_filter {
    void **vtable;
    void *skip;
};

struct cplane_t {
    Vec3 normal;
    float dist;
    unsigned char type;
    unsigned char signbits;
    unsigned char pad[2];
};

struct csurface_t {
    const char *name;
    short surface_props;
    unsigned short flags;
};

struct trace_t {
    Vec3 startpos;
    Vec3 endpos;
    cplane_t plane;
    float fraction;
    int contents;
    unsigned short disp_flags;
    bool all_solid;
    bool start_solid;
    float fraction_left_solid;
    csurface_t surface;
    int hit_group;
    short physics_bone;
    void *entity;
    int hitbox;

    // bool did_hit() const { return m_fraction < 1.f || m_allsolid || m_start_solid; }
};

#define	CONTENTS_EMPTY			0		// No contents

#define	CONTENTS_SOLID			0x1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x2		// translucent, but not watery (glass)
#define	CONTENTS_AUX			0x4
#define	CONTENTS_GRATE			0x8		// alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
#define	CONTENTS_SLIME			0x10
#define	CONTENTS_WATER			0x20
#define	CONTENTS_BLOCKLOS		0x40	// block AI line of sight
#define CONTENTS_OPAQUE			0x80	// things that cannot be seen through (may be non-solid though)
#define	LAST_VISIBLE_CONTENTS	0x80

#define ALL_VISIBLE_CONTENTS (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define CONTENTS_TESTFOGVOLUME	0x100
#define CONTENTS_UNUSED			0x200

// unused
// NOTE: If it's visible, grab from the top + update LAST_VISIBLE_CONTENTS
// if not visible, then grab from the bottom.
#define CONTENTS_UNUSED6		0x400

#define CONTENTS_TEAM1			0x800	// per team contents used to differentiate collisions
#define CONTENTS_TEAM2			0x1000	// between players and objects on different teams
#define CONTENTS_REDTEAM		CONTENTS_TEAM1
#define CONTENTS_BLUETEAM		CONTENTS_TEAM2

// ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW
#define CONTENTS_IGNORE_NODRAW_OPAQUE	0x2000

// hits entities which are MOVETYPE_PUSH (doors, plats, etc.)
#define CONTENTS_MOVEABLE		0x4000

// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEBRIS			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000
#define CONTENTS_HITBOX			0x40000000	// use accurate hitboxes on trace
#define CONTENTS_NOSTARTSOLID	0x80000000	// don't skip entities or displacements when starting in solid

#define	MASK_SOLID					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)

//add a namespace or class for these functions and vars
bool should_hit_entity(trace_filter *interface, void *entity, int contents_mask) {
    return entity != interface->skip;
}

trace_type_t trace_type = TRACE_EVERYTHING;

void set_type(trace_type_t new_trace_type) {
    trace_type = new_trace_type;
}

trace_type_t get_type(trace_filter *interface) {
    return trace_type;
}

static void *trace_filter_vtable[2] = {(void *) should_hit_entity, (void *) get_type};


class EngineTrace {
public:
    static Vec3_aligned Vec3_aligned_subtract(Vec3 *a, Vec3 *b) {
        const Vec3_aligned result = {
            .x = a->x - b->x,
            .y = a->y - b->y,
            .z = a->z - b->z
        };

        return result;
    }

    static Vec3_aligned Vec3_aligned_add(Vec3 *a, Vec3 *b) {
        const Vec3_aligned result = {
            .x = a->x + b->x,
            .y = a->y + b->y,
            .z = a->z + b->z
        };

        return result;
    }


    static ray_t init_ray(Vec3 *start, Vec3 *end) {
        Vec3_aligned delta = Vec3_aligned_subtract(end, start);
        const bool is_swept = delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f;

        const ray_t ray = {
            .start = {start->x, start->y, start->z},
            .delta = {delta.x, delta.y, delta.z},
            .start_offset = {0.0f, 0.0f, 0.0f},
            .extents = {0.0f, 0.0f, 0.0f},
            .is_ray = true,
            .is_swept = is_swept
        };

        return ray;
    }

    static ray_t init_ray(Vec3 *start, Vec3 *end, Vec3 *mins, Vec3 *maxs) {
        Vec3_aligned delta = Vec3_aligned_subtract(end, start);
        const bool is_swept = delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f;

        ray_t ray = {
            .start = {0, 0, 0},
            .delta = {delta.x, delta.y, delta.z},
            .start_offset = Vec3_aligned_add(mins, maxs),
            .extents = Vec3_aligned_subtract(maxs, mins),
            .is_ray = true,
            .is_swept = is_swept
        };

        ray.extents.x *= 0.5f;
        ray.extents.y *= 0.5f;
        ray.extents.z *= 0.5f;

        ray.start_offset.x *= 0.5f;
        ray.start_offset.y *= 0.5f;
        ray.start_offset.z *= 0.5f;

        Vec3 start_offset_tmp = Vec3{ray.start_offset.x, ray.start_offset.y, ray.start_offset.z};
        ray.start = Vec3_aligned_add(start, &start_offset_tmp);

        ray.start_offset.x *= -1.0f;
        ray.start_offset.y *= -1.0f;
        ray.start_offset.z *= -1.0f;

        return ray;
    }

    static void init_trace_filter(trace_filter *filter, void *skip) {
        filter->vtable = trace_filter_vtable;
        filter->skip = skip;
    }

    void trace_ray(ray_t *ray, unsigned int f_mask, trace_filter *p_trace_filter,
                   trace_t *p_trace) {
        void **vtable = *reinterpret_cast<void ***>(this);
        void (*trace_ray_fn)(void *, ray_t *, unsigned int, trace_filter *, trace_t *) =
                (void (*)(void *, struct ray_t *, unsigned int, struct trace_filter *, struct trace_t *)) vtable[4];

        trace_ray_fn(this, ray, f_mask, p_trace_filter, p_trace);
    }

    void trace_hull(Vec3 start, Vec3 end, Vec3 hull_min, Vec3 hull_max, const unsigned int mask, trace_t *trace) {
        ray_t ray = init_ray(&start, &end, &hull_min, &hull_max);
        trace_filter filter{};

        this->trace_ray(&ray, mask, &filter, trace);
    }
};

inline static EngineTrace *engine_trace;

#endif

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <string>
#include <string.h>
#include <cstdint>

#include "../vec.hpp"

#include "../interfaces/entity_list.hpp"

// https://github.com/rei-2/Amalgam/blob/c1c6bf64d739538b48a301ddc5e1a988cb9b479c/Amalgam/src/SDK/Definitions/Definitions.h#L1032
enum class_id {  
  AMMO_OR_HEALTH_PACK = 1,
  DISPENSER = 86,
  SENTRY = 88,
  TELEPORTER = 89,
  ARROW = 122,
  PLAYER = 247,
  ROCKET = 264,
  PILL_OR_STICKY = 217,
  FLARE = 257,
  CROSSBOW_BOLT = 259,
  SNIPER_DOT = 118,
  CAPTURE_FLAG = 26,
  OBJECTIVE_RESOURCE = 235,
  WEARABLE_VM = 343,
  WEARABLE = 336,
  WEARABLE_ITEM = 339,
  WEARABLE_ECON = 35,
  WEARABLE_ROBOT_ARM = 342,
  WEARABLE_RAZORBACK = 341,
  WEARABLE_DEMO_SHIELD = 338,
  WEARABLE_LEVELABLE_ITEM = 340,
  WEARABLE_CAMPAIGN_ITEM = 337,
  RESPAWN_ROOM_VISUALIZER = 64,
  BASE_DOOR = 6,
  BASE_PROP_DOOR = 15,
  
  // Negative values are reserved by the hack for entity_cache.hpp
  // Entity::get_class_id() will not return any of these.
  AMMO = -2,
  HEALTH_PACK = -1,
};

enum class tf_team {
  UNKNOWN = 0,
  SPECTATOR,
  RED,
  BLU
};

enum pickup_type {
  UNKNOWN = 0,
  MEDKIT,
  AMMOPACK,
};

enum entity_flags {
    FL_ONGROUND = (1 << 0),
    FL_DUCKING = (1 << 1),
    FL_WATERJUMP = (1 << 2),
    FL_ONTRAIN = (1 << 3),
    FL_INRAIN = (1 << 4),
    FL_FROZEN = (1 << 5),
    FL_ATCONTROLS = (1 << 6),
    FL_CLIENT = (1 << 7),
    FL_FAKECLIENT = (1 << 8),
    FL_INWATER = (1 << 9),
    FL_FLY = (1 << 10),
    FL_SWIM = (1 << 11),
    FL_CONVEYOR = (1 << 12),
    FL_NPC = (1 << 13),
    FL_GODMODE = (1 << 14),
    FL_NOTARGET = (1 << 15),
    FL_AIMTARGET = (1 << 16),
    FL_PARTIALGROUND = (1 << 17),
    FL_STATICPROP = (1 << 18),
    FL_GRAPHED = (1 << 19),
    FL_GRENADE = (1 << 20),
    FL_STEPMOVEMENT = (1 << 21),
    FL_DONTTOUCH = (1 << 22),
    FL_BASEVELOCITY = (1 << 23),
    FL_WORLDBRUSH = (1 << 24),
    FL_OBJECT = (1 << 25),
    FL_KILLME = (1 << 26),
    FL_ONFIRE = (1 << 27),
    FL_DISSOLVING = (1 << 28),
    FL_TRANSRAGDOLL = (1 << 29),
    FL_UNBLOCKABLE_BY_PLAYER = (1 << 30)
};

#define STUDIO_NONE 0x00000000
#define STUDIO_RENDER 0x00000001
#define STUDIO_VIEWXFORMATTACHMENTS 0x00000002
#define STUDIO_DRAWTRANSLUCENTSUBMODELS 0x00000004
#define STUDIO_TWOPASS 0x00000008
#define STUDIO_STATIC_LIGHTING 0x00000010
#define STUDIO_WIREFRAME 0x00000020
#define STUDIO_ITEM_BLINK 0x00000040
#define STUDIO_NOSHADOWS 0x00000080
#define STUDIO_WIREFRAME_VCOLLIDE 0x00000100
#define STUDIO_NO_OVERRIDE_FOR_ATTACH 0x00000200
#define STUDIO_GENERATE_STATS 0x01000000
#define STUDIO_SSAODEPTHTEXTURE 0x08000000
#define STUDIO_SHADOWDEPTHTEXTURE 0x40000000
#define STUDIO_TRANSPARENCY 0x80000000

class Entity {
public:  
  int get_owner_entity_handle(void) {
    return *(int*)(this + 0x754);
  }

  Entity* get_owner_entity(void) {
    return entity_list->entity_from_handle(this->get_owner_entity_handle());
  }
  
  Vec3 get_origin(void) {
    // x + 0x328, y + 0x332, z + 0x346
    return *(Vec3*)(this + 0x328);
  }

  int get_ent_flags(void) {
    return *(int*)(this + 0x460);
  }
  
  // TODO: Substitute "void*" with real class type
  void* get_networkable(void) {
    return (void*)(this + 0x10);
  }

  void* get_renderable(void) {
    return (void*)(this + 0x8);
  }

  enum tf_team get_team(void)  {
    return (enum tf_team)*(int*)(this + 0xDC);
  }
  
  int get_index(void) {
    void* networkable = get_networkable();
    void** vtable = *(void***)networkable;

    int (*get_index_fn)(void*) = (int (*)(void*))vtable[9];

    return get_index_fn(networkable);
  }
  
  const char* get_model_name(void) {
    uintptr_t base_class = *(uintptr_t*)(this + 0x88);
    if (base_class == NULL) return "";
    
    return (const char*)*(unsigned long*)(base_class + 0x8);
  }
  
  int draw_model(int flags) {
    void* renderable = this->get_renderable();
    void** vtable = *(void***)renderable;

    int (*draw_model_fn)(void*, int) = (int (*)(void*, int))vtable[10];

    return draw_model_fn(renderable, flags);
  }
  
  bool is_dormant(void) {
    void* networkable = get_networkable();
    void** vtable = *(void ***)networkable;

    bool (*is_dormant_fn)(void*) = (bool (*)(void*))vtable[8];

    return is_dormant_fn(networkable);
  }  
  
  void* get_client_class(void) {
    void* networkable = get_networkable();
    void** vtable = *(void ***)networkable;

    void* (*get_client_class_fn)(void*) = (void* (*)(void*))vtable[2];

    return get_client_class_fn(networkable);
  }
  
  class_id get_class_id(void) {
    void* client_class = get_client_class();
    return *(class_id*)((unsigned long)(client_class) + 0x28);
  }

  int get_tickbase(void) {
    return *(int*)(this + 0x1718);
  }

  void set_tickbase(int tickbase) {
    *(int*)(this + 0x1718) = tickbase;
  }

  
  float get_simulation_time(void) {
    return *(float*)(this + 0x98);
  }
  
  bool is_building(void) {
    switch (this->get_class_id()) {
    case class_id::SENTRY:
    case class_id::DISPENSER:
    case class_id::TELEPORTER:
      return true;
    }

    return false;
  }

  bool is_wearable(void) {
    switch (this->get_class_id()) {
    case class_id::WEARABLE:
    case class_id::WEARABLE_CAMPAIGN_ITEM:
    case class_id::WEARABLE_DEMO_SHIELD:
    case class_id::WEARABLE_ECON:
    case class_id::WEARABLE_ITEM:
    case class_id::WEARABLE_RAZORBACK:
    case class_id::WEARABLE_VM:
    case class_id::WEARABLE_LEVELABLE_ITEM:
      return true;
    }

    return false;
  }
  
  enum pickup_type get_pickup_type(void) {
    const char* model_name = get_model_name();
    
    if (strstr(model_name, "models/items/ammopack")) {
      return pickup_type::AMMOPACK;
    }

    if (strstr(model_name, "models/items/medkit")                     ||
	strstr(model_name, "models/props_medieval/medieval_meat.mdl") ||
	strstr(model_name, "models/props_halloween/halloween_medkit")
	)
      {
	return pickup_type::MEDKIT;
      }

    return pickup_type::UNKNOWN;
  }

  bool is_base_combat_weapon(void) {
    void** vtable = *(void***)this;

    bool (*is_base_combat_weapon_fn)(void*) = (bool (*)(void*))vtable[138];

    return is_base_combat_weapon_fn(this);
  }
  
};

#endif

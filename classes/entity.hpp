#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <string>
#include <string.h>

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
  SNIPER_DOT = 118
};

enum pickup_type {
  UNKNOWN,
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

  int get_index(void) {
    void* networkable = get_networkable();
    void** vtable = *(void***)networkable;

    int (*get_index_fn)(void*) = (int (*)(void*))vtable[9];

    return get_index_fn(networkable);
  }
  
  const char* get_model_name(void) {
    unsigned long a = *(unsigned long*)(this + 0x88);
    if (!a) return "";
    unsigned long b = *(unsigned long*)(a + 0x8);
    if (!b) return "";
    return (const char*)b;
  }

  enum pickup_type get_pickup_type(void) {
    const char* model_name = get_model_name();
    
    if (strstr(model_name, "models/items/ammopack")) {
      return pickup_type::AMMOPACK;
    }

    if (strstr(model_name, "models/items/medkit") || strstr(model_name, "models/props_medieval/medieval_meat.mdl")) {
      return pickup_type::MEDKIT;
    }

    return pickup_type::UNKNOWN;
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
  
  int get_class_id(void) {
    void* client_class = get_client_class();
    return *(int*)((unsigned long)(client_class) + 0x28);
  }

  std::string get_class_id_to_string(void) {
    // TBD: Check model name for more accurate class name
    switch (get_class_id()) {
    case AMMO_OR_HEALTH_PACK:
      return "AMMO/HP";
    case DISPENSER:
      return "DISPENSER";
    case SENTRY:
      return "SENTRY";
    case TELEPORTER:
      return "TELEPORTER";
    case ARROW:
      return "ARROW";
    case PLAYER:
      return "PLAYER";
    case ROCKET:
      return "ROCKET";
    case PILL_OR_STICKY:
      return "NADE";
    case FLARE:
      return "FLARE";
    case CROSSBOW_BOLT:
      return "BOLT";
    default:
      return "NOT IMPLEMENTED";
    }
  }

  int get_tickbase(void) {
    return *(int*)(this + 0x1718);
  }

};

#endif

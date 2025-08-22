#include <cmath>
#include "../../gui/config.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/engine.hpp"
#include "../../classes/player.hpp"
#include "../../classes/weapon.hpp"
#include "../../print.hpp"

struct TargetCandidate;
typedef int (*BonePickerFn)(Player* local, Player* target, Weapon* weapon);
static TargetCandidate find_best_target(Player* localplayer, Weapon* weapon, const Vec3& original_view_angle, float max_fov, bool friendlyfire, bool ignore_friends, BonePickerFn bone_picker);
static inline bool aim_key_active();
static inline int bone_from_selector(Player* local, Player* target, int selector);

static SniperDot* find_local_sniper_dot(Player* localplayer) {
  if (!localplayer) return nullptr;
  int max_ents = entity_list->get_max_entities();
  for (int i = 0; i < max_ents; i++) {
    Entity* ent = entity_list->entity_from_index(i);
    if (!ent) continue;
    if (ent->is_dormant()) continue;
    if (ent->get_class_id() != SNIPER_DOT) continue;
    if (ent->get_owner_entity() == localplayer) {
      return reinterpret_cast<SniperDot*>(ent);
    }
  }
  return nullptr;
}

static int hitscan_bone_picker(Player* local, Player* target, Weapon* weapon) {
  (void)weapon;
  return bone_from_selector(local, target, config.aimbot.hitscan.hitbox);
}

static bool aimbot_hitscan(Player* localplayer, Weapon* weapon, user_cmd* user_cmd, const Vec3& original_view_angle, bool friendlyfire) {
  if (!config.aimbot.hitscan.enabled)
    return false;

  if ((config.aimbot.hitscan.modifiers & Aim::Hitscan::ScopedOnly) && localplayer->get_tf_class() == CLASS_SNIPER && !localplayer->is_scoped())
    return false;

  TargetCandidate best = find_best_target(localplayer, weapon, original_view_angle, config.aimbot.fov, friendlyfire, config.aimbot.ignore_friends, hitscan_bone_picker);

  if (!best.player) {
    if (target_player) target_player = nullptr;
    return false;
  }

  target_player = best.player;

  if (aim_key_active() && config.aimbot.auto_shoot && target_player == best.player && localplayer->can_shoot()) {
    bool gate_headshot = (config.aimbot.hitscan.modifiers & Aim::Hitscan::WaitForHeadshot) != 0;
    bool allow_fire = true;

    if (gate_headshot && weapon) {
      int type_id = weapon->get_type_id();
      short def_id = weapon->get_def_id();

      if (weapon->is_sniper_rifle()) {
        if (!localplayer->is_scoped()) {
          allow_fire = false;
        } else {
          SniperDot* dot = find_local_sniper_dot(localplayer);
          if (!dot || !weapon->can_sniper_rifle_headshot(dot)) {
            allow_fire = false; // wait until we can headshot
          }
        }
      }

      if (def_id == Spy_m_TheAmbassador || def_id == Spy_m_FestiveAmbassador) {
        if (!weapon->can_ambassador_headshot()) {
          allow_fire = false;
        }
      }

    }

    if (best.player && best.player->is_invulnerable()) {
      allow_fire = false;
    }

    if (allow_fire) {
      user_cmd->buttons |= IN_ATTACK;
      is_shooting = true;
    }
  }

  if (aim_key_active() && weapon->can_primary_attack() && target_player == best.player)
    user_cmd->view_angles = best.aim_angles;

  return true;
}

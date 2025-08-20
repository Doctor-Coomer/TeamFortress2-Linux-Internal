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

static int projectile_bone_picker(Player* local, Player* target, Weapon* weapon) {
  (void)weapon;
  return bone_from_selector(local, target, config.aimbot.projectile.hitbox);
}


// we dont really need proj aim for bots lol
static bool aimbot_projectile(Player* localplayer, Weapon* weapon, user_cmd* user_cmd, const Vec3& original_view_angle, bool friendlyfire) {
  if (!config.aimbot.projectile.enabled)
    return false;

  TargetCandidate best = find_best_target(localplayer, weapon, original_view_angle, config.aimbot.fov, friendlyfire, config.aimbot.ignore_friends, projectile_bone_picker);

  if (!best.player) {
    if (target_player) target_player = nullptr;
    return false;
  }

  target_player = best.player;

  if (aim_key_active() && config.aimbot.auto_shoot && target_player == best.player && localplayer->can_shoot()) {
    if (best.player && !best.player->is_invulnerable()) {
      user_cmd->buttons |= IN_ATTACK;
      is_shooting = true;
    }
  }

  if (aim_key_active() && weapon->can_primary_attack() && target_player == best.player)
    user_cmd->view_angles = best.aim_angles;

  return true;
}

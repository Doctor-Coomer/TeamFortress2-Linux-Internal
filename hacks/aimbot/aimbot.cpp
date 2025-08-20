#include "aimbot.hpp"

#include <cmath>
#include <cfloat>
#include <cstring>
#include <limits>
#include <unordered_map>

#include "aimbot_hitscan.cpp"
#include "aimbot_projectile.cpp"
#include "aimbot_melee.cpp"

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/engine.hpp"
#include "../../interfaces/engine_trace.hpp"
#include "../../interfaces/convar_system.hpp"

#include "../../gui/config.hpp"
#include "../../classes/player.hpp"
#include "../../classes/weapon.hpp"

#include "../../hooks/weapon_groups.hpp"

#include "../../print.hpp"

bool is_shooting = false;

bool is_player_visible(Player* localplayer, Player* entity, int bone) {
  Vec3 target_pos = entity->get_bone_pos(bone);
  Vec3 start_pos  = localplayer->get_shoot_pos();
  
  struct ray_t ray = engine_trace->init_ray(&start_pos, &target_pos);
  struct trace_filter filter;
  engine_trace->init_trace_filter(&filter, localplayer);
  
  struct trace_t trace;
  engine_trace->trace_ray(&ray, 0x4200400b, &filter, &trace);
  
  if (trace.entity == entity || trace.fraction > 0.97f) {
    return true;
  }

  return false;
}

void movement_fix(user_cmd* user_cmd, Vec3 original_view_angle, float original_forward_move, float original_side_move) {
  float yaw_delta = user_cmd->view_angles.y - original_view_angle.y;
  float original_yaw_correction = 0;
  float current_yaw_correction = 0;

  if (original_view_angle.y < 0.0f) {
    original_yaw_correction = 360.0f + original_view_angle.y;
  } else {
    original_yaw_correction = original_view_angle.y;
  }
    
  if (user_cmd->view_angles.y < 0.0f) {
    current_yaw_correction = 360.0f + user_cmd->view_angles.y;
  } else {
    current_yaw_correction = user_cmd->view_angles.y;
  }

  if (current_yaw_correction < original_yaw_correction) {
    yaw_delta = abs(current_yaw_correction - original_yaw_correction);
  } else {
    yaw_delta = 360.0f - abs(original_yaw_correction - current_yaw_correction);
  }
  yaw_delta = 360.0f - yaw_delta;

  user_cmd->forwardmove = cos((yaw_delta) * (M_PI/180)) * original_forward_move + cos((yaw_delta + 90.f) * (M_PI/180)) * original_side_move;
  user_cmd->sidemove = sin((yaw_delta) * (M_PI/180)) * original_forward_move + sin((yaw_delta + 90.f) * (M_PI/180)) * original_side_move;
}

 static inline void get_head_body_bones(Player* target, int& head_bone, int& body_bone) {
  head_bone = target->get_head_bone();
  body_bone = 2;
}

enum HitboxMask {
  HB_Head   = 1 << 0,
  HB_Body   = 1 << 1,
  HB_Pelvis = 1 << 2,
  HB_Arms   = 1 << 3,
  HB_Legs   = 1 << 4,
};

static bool is_hitbox_valid(Player* target, int nHitbox /*bone index*/, int mask) {
  if (!target)
    return false;
  if (nHitbox == -1)
    return true;

  int head_bone = 0, body_bone = 2;
  get_head_body_bones(target, head_bone, body_bone);

  if ((mask & HB_Head) && nHitbox == head_bone)
    return true;

  if ((mask & HB_Body) && nHitbox == body_bone)
    return true;

  return false;
}

struct TargetCandidate {
  Player* player;
  Vec3 aim_angles;
  float fov;
  int bone;
  bool visible;
};

typedef int (*BonePickerFn)(Player* local, Player* target, Weapon* weapon);

static inline bool aim_key_active() {
  return ((is_button_down(config.aimbot.key) && config.aimbot.use_key) || !config.aimbot.use_key);
}

static inline int bone_from_selector(Player* local, Player* target, int selector) {
  (void)local;
  int head_bone = 0, body_bone = 2;
  get_head_body_bones(target, head_bone, body_bone);
  return selector == 1 ? head_bone : body_bone;
}

static TargetCandidate find_best_target(Player* localplayer, Weapon* weapon, const Vec3& original_view_angle, float max_fov, bool friendlyfire, bool ignore_friends, BonePickerFn bone_picker) {
  TargetCandidate best{nullptr, {}, FLT_MAX, 2, false};

  for (unsigned int i = 1; i <= entity_list->get_max_entities(); ++i) {
    Player* player = entity_list->player_from_index(i);
    if (player == nullptr ||
        player == localplayer ||
        player->is_dormant() == true ||
        (player->get_team() == localplayer->get_team() && friendlyfire == false) ||
        player->get_lifestate() != 1 ||
        player->is_invulnerable() == true ||
        (ignore_friends == true && player->is_friend())) {
      continue;
    }

    int bone = bone_picker ? bone_picker(localplayer, player, weapon) : 2;

    Vec3 target = player->get_bone_pos(bone);
    Vec3 src = localplayer->get_shoot_pos();
    Vec3 diff = { target.x - src.x, target.y - src.y, target.z - src.z };

    float yaw_hyp = sqrtf((diff.x * diff.x) + (diff.y * diff.y));
    float pitch_angle = atan2f(diff.z, yaw_hyp) * 180.0f / (float)M_PI;
    float yaw_angle = atan2f(diff.y, diff.x) * 180.0f / (float)M_PI;

    Vec3 view_angles = { -pitch_angle, yaw_angle, 0.0f };

    float x_diff = view_angles.x - original_view_angle.x;
    float y_diff = view_angles.y - original_view_angle.y;
    float x = remainderf(x_diff, 360.0f);
    float y = remainderf(y_diff, 360.0f);
    float clamped_x = x > 89.0f ? 89.0f : x < -89.0f ? -89.0f : x;
    float clamped_y = y > 180.0f ? 180.0f : y < -180.0f ? -180.0f : y;
    float fov = hypotf(clamped_x, clamped_y);

    bool visible = is_player_visible(localplayer, player, bone);

    if (visible && fov <= max_fov && fov < best.fov) {
      best.player = player;
      best.aim_angles = view_angles;
      best.fov = fov;
      best.bone = bone;
      best.visible = true;
    }
  }

  return best;
}

void aimbot(user_cmd* user_cmd) {
  is_shooting = false;
  if (!config.aimbot.master) {
    target_player = nullptr;
    return;
  }

  Player* localplayer = entity_list->get_localplayer();
  if (localplayer->get_lifestate() != 1) {
    target_player = nullptr;
    return;
  }

  Weapon* weapon = localplayer->get_weapon();
  if (weapon == nullptr) {
    target_player = nullptr;
    return;
  }

  if (target_player != nullptr && target_player->is_friend() && config.aimbot.ignore_friends) {
    target_player = nullptr;
  }

  Vec3 original_view_angle = user_cmd->view_angles;
  float original_side_move = user_cmd->sidemove;
  float original_forward_move = user_cmd->forwardmove;

  bool friendlyfire = false;
  static Convar* friendlyfirevar = convar_system->find_var("mp_friendlyfire");
  if (friendlyfirevar != nullptr && friendlyfirevar->get_int() != 0) {
    friendlyfire = true;
  }

  static int last_type_id = -1;
  static int last_def_id = -1;
  static int last_dispatch = -1; // 0 = melee, 1 = projectile, 2 = hitscan

  int type_id = weapon->get_type_id();
  int def_id = weapon->get_def_id();
  bool is_melee_weapon = weapon_groups::is_melee_type_id(type_id)
    || weapon_groups::is_melee_def_id(def_id)
    || weapon_groups::is_spy_knife_def_id(def_id);
  bool is_projectile_w = weapon_groups::is_projectile_type_id(type_id);
  int dispatch_decision = is_melee_weapon ? 0 : (is_projectile_w ? 1 : 2);

  if (type_id != last_type_id || def_id != last_def_id || dispatch_decision != last_dispatch) {
    void** vtable = *(void***)weapon;
    void* get_type_fn = vtable[449];
    print("[aimbot] weapon=%p type_id=%d def_id=%d melee=%d projectile=%d dispatch=%s vtable=%p get_type_id_fn=%p\n",
          weapon,
          type_id,
          def_id,
          (int)is_melee_weapon,
          (int)is_projectile_w,
          dispatch_decision == 0 ? "melee" : (dispatch_decision == 1 ? "projectile" : "hitscan"),
          vtable,
          get_type_fn);
    last_type_id = type_id;
    last_def_id = def_id;
    last_dispatch = dispatch_decision;
  }

  bool handled = false;
  if (is_melee_weapon) {
    if (config.aimbot.melee.enabled) {
      handled = aimbot_melee(localplayer, weapon, user_cmd, original_view_angle, friendlyfire);
    }
  } else if (is_projectile_w) {
    handled = aimbot_projectile(localplayer, weapon, user_cmd, original_view_angle, friendlyfire);
  } else {
    handled = aimbot_hitscan(localplayer, weapon, user_cmd, original_view_angle, friendlyfire);
  }

  (void)handled; // reserved for future logic
  movement_fix(user_cmd, original_view_angle, original_forward_move, original_side_move);
}

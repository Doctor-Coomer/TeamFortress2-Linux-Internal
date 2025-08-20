#include <cmath>
#include <unordered_map>
#include "../../gui/config.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/engine.hpp"
#include "../../classes/player.hpp"
#include "../../classes/weapon.hpp"
#include "../../hooks/weapon_groups.hpp"
#include "../../print.hpp"
#include "../../math.hpp"
#include "../../interfaces/engine_trace.hpp"
#include "../../interfaces/attribute_manager.hpp"

struct TargetCandidate;
typedef int (*BonePickerFn)(Player* local, Player* target, Weapon* weapon);
static TargetCandidate find_best_target(Player* localplayer, Weapon* weapon, const Vec3& original_view_angle, float max_fov, bool friendlyfire, bool ignore_friends, BonePickerFn bone_picker);
static inline bool aim_key_active();

static int melee_bone_picker(Player* local, Player* target, Weapon* weapon) {
  (void)local; (void)weapon;
  return 2; // chest
}

struct VelRecord {
  Vec3 prev_origin{};
  Vec3 velocity{};
  bool has_prev{};
};

static std::unordered_map<int, VelRecord> g_vel_map;

static inline Vec3 estimate_velocity(Player* p) {
  if (!p) return {0.f,0.f,0.f};
  int idx = p->get_index();
  Vec3 cur = p->get_origin();
  auto& rec = g_vel_map[idx];
  if (rec.has_prev) {
    rec.velocity.x = (cur.x - rec.prev_origin.x) / TICK_INTERVAL;
    rec.velocity.y = (cur.y - rec.prev_origin.y) / TICK_INTERVAL;
    rec.velocity.z = (cur.z - rec.prev_origin.z) / TICK_INTERVAL;
  }
  rec.prev_origin = cur;
  rec.has_prev = true;
  return rec.velocity;
}

static inline Vec3 angle_to(const Vec3& src, const Vec3& dst) {
  Vec3 diff{ dst.x - src.x, dst.y - src.y, dst.z - src.z };
  float hyp = sqrtf(diff.x * diff.x + diff.y * diff.y);
  float pitch = atan2f(diff.z, hyp) * 180.0f / (float)M_PI;
  float yaw   = atan2f(diff.y, diff.x) * 180.0f / (float)M_PI;
  return Vec3{ -pitch, yaw, 0.0f };
}

static inline bool is_valid_backstab(Player* local, Player* target, const Vec3& predicted_target_pos, float cone_deg) {
  if (!local || !target)
    return false;
  Vec3 eye = target->get_eye_angles();
  Vec3 fwd{};
  angle_vectors(Vec3{0.f, eye.y, 0.f}, &fwd, nullptr, nullptr);

  Vec3 lpos = local->get_shoot_pos();
  Vec3 to_local = { lpos.x - predicted_target_pos.x, lpos.y - predicted_target_pos.y, 0.f };
  float len = sqrtf(to_local.x * to_local.x + to_local.y * to_local.y);
  if (len < 1e-3f)
    return false;
  to_local.x /= len; to_local.y /= len;

  float dot = fwd.x * to_local.x + fwd.y * to_local.y;
  float cone_rad = cone_deg * (float)M_PI / 180.f;
  float cos_thresh = cosf(cone_rad);
  return dot <= -cos_thresh;
}

static inline void get_melee_params(Weapon* weapon, float& out_range, float& out_hull)
{
  float base_range = config.aimbot.melee.melee_reach;
  float base_hull  = 18.0f;

  if (weapon && attribute_hook_value_float_original) {
    out_range = attribute_manager->attrib_hook_value(base_range, "melee_range_multiplier", (Entity*)weapon);
    out_hull  = attribute_manager->attrib_hook_value(base_hull,  "melee_bounds_multiplier", (Entity*)weapon);
  } else {
    out_range = base_range;
    out_hull  = base_hull;
  }
}

static inline float dist_point_to_segment(const Vec3& a, const Vec3& b, const Vec3& p)
{
  Vec3 ab{ b.x - a.x, b.y - a.y, b.z - a.z };
  Vec3 ap{ p.x - a.x, p.y - a.y, p.z - a.z };
  float ab_len2 = ab.x*ab.x + ab.y*ab.y + ab.z*ab.z;
  if (ab_len2 <= 1e-6f) return distance_3d(a, p);
  float t = (ap.x*ab.x + ap.y*ab.y + ap.z*ab.z) / ab_len2;
  if (t < 0.f) t = 0.f; else if (t > 1.f) t = 1.f;
  Vec3 c{ a.x + ab.x*t, a.y + ab.y*t, a.z + ab.z*t };
  return distance_3d(c, p);
}

static inline bool melee_hull_can_hit(Player* local, Player* target, Weapon* weapon, const Vec3& aim_angles, const Vec3& predicted_hit_pos)
{
  if (!local || !target || !weapon || !engine_trace)
    return false;

  float range = 0.f, hull = 0.f;
  get_melee_params(weapon, range, hull);

  Vec3 start = local->get_shoot_pos();
  Vec3 fwd{}; angle_vectors(aim_angles, &fwd, nullptr, nullptr);
  Vec3 end{ start.x + fwd.x * range, start.y + fwd.y * range, start.z + fwd.z * range };

  Vec3 mins{ -hull, -hull, -hull };
  Vec3 maxs{  hull,  hull,  hull };

  trace_filter filter{};
  engine_trace->init_trace_filter(&filter, (void*)local);

  ray_t ray = engine_trace->init_ray(&start, &end, &mins, &maxs);
  trace_t tr{};
  engine_trace->trace_ray(&ray, MASK_SOLID | CONTENTS_HITBOX, &filter, &tr);

  if (tr.entity == (void*)target)
    return true;

  if (tr.fraction >= 1.0f) {
    float d = dist_point_to_segment(start, end, predicted_hit_pos);
    if (d <= hull)
      return true;
  }

  return false;
}

static bool aimbot_melee(Player* localplayer, Weapon* weapon, user_cmd* user_cmd, const Vec3& original_view_angle, bool friendlyfire) {
  if (!config.aimbot.melee.enabled)
    return false;

  TargetCandidate best = find_best_target(localplayer, weapon, original_view_angle, config.aimbot.fov, friendlyfire, config.aimbot.ignore_friends, melee_bone_picker);

  if (!best.player) {
    if (target_player) target_player = nullptr;
    return false;
  }

  target_player = best.player;

  float time_now = localplayer->get_tickbase() * TICK_INTERVAL;
  float next_attack = weapon->get_next_attack();
  float next_primary = weapon->get_next_primary_attack();
  float t_ready = std::fmax(0.f, std::fmax(next_attack, next_primary) - time_now);
  float lead_time = config.aimbot.melee.predict ? (t_ready + config.aimbot.melee.predict_time) : 0.0f;

  Vec3 aim_angles = best.aim_angles;
  Vec3 predicted_hit_pos = best.player->get_bone_pos(best.bone);
  if (config.aimbot.melee.predict) {
    Vec3 vel = estimate_velocity(best.player);
    predicted_hit_pos.x += vel.x * lead_time;
    predicted_hit_pos.y += vel.y * lead_time;
    predicted_hit_pos.z += vel.z * lead_time;
    aim_angles = angle_to(localplayer->get_shoot_pos(), predicted_hit_pos);
  }

  if (aim_key_active() && target_player == best.player) {
    if (weapon->can_primary_attack()) {
      user_cmd->view_angles = aim_angles;
    } else {
      user_cmd->view_angles = aim_angles;
    }
  }

  if (aim_key_active() && config.aimbot.auto_shoot && target_player == best.player) {
    if (best.player && best.player->is_invulnerable()) {
      return true;
    }
    bool should_attack = localplayer->can_shoot();

    bool in_range_at_swing = melee_hull_can_hit(localplayer, best.player, weapon, aim_angles, predicted_hit_pos);
    bool ready_or_soon = weapon->can_primary_attack() || (t_ready <= TICK_INTERVAL);
    should_attack = should_attack && in_range_at_swing && ready_or_soon;

    {
      float rng = 0.f, hull = 0.f;
      get_melee_params(weapon, rng, hull);
      float dist_now = distance_3d(localplayer->get_shoot_pos(), predicted_hit_pos);
      constexpr float kTriggerFrac = 0.90f;
      bool close_enough = dist_now <= (rng * kTriggerFrac);
      should_attack = should_attack && close_enough;
    }

    // its bad, meleeaimbot is bad at all
    int t = weapon->get_type_id();
    int d = weapon->get_def_id();
    bool is_knife = weapon_groups::is_knife_type_id(t) || weapon_groups::is_spy_knife_def_id(d);
    if (config.aimbot.melee.auto_backstab && localplayer->get_tf_class() == CLASS_SPY && is_knife) {
      bool valid_backstab = is_valid_backstab(localplayer, best.player, predicted_hit_pos, config.aimbot.melee.backstab_cone_deg);
      should_attack = should_attack && valid_backstab;
    }

    if (should_attack) {
      user_cmd->buttons |= IN_ATTACK;
      is_shooting = true;
    }
  }

  return true;
}

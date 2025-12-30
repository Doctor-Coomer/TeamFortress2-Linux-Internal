#include <vector>
#include <string>
#include <algorithm> 
#include <functional> 

#include "../../interfaces/client.hpp"
#include "../../interfaces/netvars.hpp" 
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/engine_trace.hpp"

#include "../../gui/config.hpp"
#include "../../classes/player.hpp"

#include "../../print.hpp"
#include "../../vec.hpp"
#include "../../math.hpp"

constexpr float MAX_PROJECTILE_PREDICTION_TIME = 1.5f;

bool IsPointVisible(const Vec3& start, const Vec3& end, Entity* skip_entity) {
  trace_t tr;
  trace_filter filter;

  engine_trace->init_trace_filter(&filter, skip_entity);
  ray_t ray = engine_trace->init_ray((Vec3*)&start, (Vec3*)&end);
  engine_trace->trace_ray(&ray, 0x4200400b, &filter, &tr); 
  
  return (tr.fraction > 0.97f);
}


static bool IsSniperAimingAt(Player* sniper, const Vec3& target_pos, float dot_threshold) {
  Vec3 sniper_eye_pos = sniper->get_shoot_pos();
  if (!IsPointVisible(sniper_eye_pos, target_pos, sniper->to_entity())) {
    return false;
  }

  Vec3 sniper_forward_dir;
  angle_vectors(sniper->get_eye_angles(), &sniper_forward_dir, nullptr, nullptr);

  Vec3 vector_to_target = target_pos - sniper_eye_pos;
  vector_to_target.normalize();

  return vector_to_target.dot(sniper_forward_dir) > dot_threshold;
}


// TODO: add headshot weapons, like ambassador
// and maybe add backtrack, if sniper headshot -> turn uber
// the machina checking penetration (idk, machina have other hitboxes head? idk)
int BulletDangerValue(Player* player) {
    if (!player || player->is_dead()) return 0;

    std::vector<int> bones_to_check = {
        player->get_head_bone(), // head
        player->get_neck_bone(), // neck
        4,                       // spine_3 
        3,                       // spine_2 
        2,                       // spine_1 
        0,                       // pelvis 
        8,                       // arm_upper_L 
        9,                       // arm_lower_L
        11,                      // arm_upper_R 
        12,                      // arm_lower_R 
        13,                      // leg_upper_L
        14,                      // leg_lower_L
        16,                      // leg_upper_R
        17                       // leg_lower_R 
    };
    
    for (int i = 1; i <= entity_list->get_max_entities(); ++i) {
        Player* sniper = entity_list->player_from_index(i);

        if (!sniper || sniper == player || sniper->is_dead() || sniper->get_team() == player->get_team() || sniper->get_tf_class() != CLASS_SNIPER) {
            continue;
        }
    
        if (sniper->in_cond(TF_COND_AIMING)) {
            for (int bone_index : bones_to_check) {
                Vec3 bone_pos = player->get_bone_pos(bone_index);

                if (bone_pos.x == 0 && bone_pos.y == 0) {
                    continue;
                }
                
                float dot_threshold = (bone_index == player->get_head_bone()) ? 0.999f : 0.995f;

                if (IsSniperAimingAt(sniper, bone_pos, dot_threshold)) {
                    return 2;
                }
            }
        }
    }

    return 0;
}


// TODO: if a flare flying to player and the player is on fire, turn on uber
int FireDangerValue(Player* player) {
  if (player->in_cond(TF_COND_BURNING)) {
  float health_threshold = player->get_max_health() * 0.65f; 
  return (player->get_health() < health_threshold) ? 2 : 1;

  } return 0;
}


bool IsProjectileOnCollisionCourse(Player* target, Vec3 projectile_pos, Vec3 projectile_vel, float projectile_gravity, float max_sim_time, float splash_radius, float time_step = 0.05f) {
  if (!target) return false;

  float splash_radius_sq = splash_radius * splash_radius;

  Vec3 player_origin = target->get_origin();
  Vec3 mins = player_origin - Vec3{20.0f, 20.0f, 0.0f};
  Vec3 maxs = player_origin + Vec3{20.0f, 20.0f, target->is_ducking() ? 56.0f : 72.0f};
  Vec3 gravity_vec = {0, 0, -800.0f * projectile_gravity};

  for (float t = 0.0f; t < max_sim_time; t += time_step) {
    projectile_pos = projectile_pos + (projectile_vel * time_step);
    projectile_vel = projectile_vel + (gravity_vec * time_step);
  
    if (projectile_pos.x > mins.x && projectile_pos.x < maxs.x &&
      projectile_pos.y > mins.y && projectile_pos.y < maxs.y &&
      projectile_pos.z > mins.z && projectile_pos.z < maxs.z) {
      return true;
    }

    if (projectile_pos.z < player_origin.z + 10.0f) {
      if (dist_sqr(player_origin, projectile_pos) < splash_radius_sq) {
        return true;
      }
    }
  }
  return false;
}

//idk why not for demo pipes, need fix
int BlastDangerValue(Player* patient) {
  if (!patient || patient->get_health() <= 0) return 0;

  int total_danger_score = 0;
  int nearby_armed_stickies = 0;
  Vec3 patient_pos = patient->get_origin();

  for (int i = 1; i <= entity_list->get_max_entities(); ++i) {
    Entity* ent = entity_list->entity_from_index(i);

    if (!ent || ent->is_dormant()) continue;
    Entity* owner = ent->get_owner_entity();
    if (!owner || owner->get_team() == patient->get_team()) continue;

    ClientClass* client_class = (ClientClass*)ent->get_client_class();
    if (!client_class || !client_class->m_pNetworkName) continue;

    const char* class_name = client_class->m_pNetworkName;

    // if (strncmp(class_name, "CTF", 3) == 0) {
    //   if (global_vars->tickcount % 60 == 0) {
    //     print("[DEBUG] Projectile Found: %s\n", class_name);
    //   }
    // }

    float splash_radius = ent->get_splash_radius();
    float splash_radius_sq = splash_radius * splash_radius;
    
    if (strcmp(class_name, "CTFProjectile_Rocket") == 0 || strcmp(class_name, "CTFProjectile_SentryRocket") == 0) {
      if (IsProjectileOnCollisionCourse(patient, ent->get_origin(), ent->get_velocity(), 0.0f, MAX_PROJECTILE_PREDICTION_TIME, splash_radius)) {
        total_danger_score += ent->is_crit_projectile() ? 100 : 50;
      }
    }

    // else if (strcmp(class_name, "CTFProjectile_Stickybomb") == 0) {
    //   bool is_armed = *(bool*)((uintptr_t)ent + 0xC55); 
    //   if (is_armed && dist_sqr(patient_pos, ent->get_origin()) < splash_radius_sq) {
    //     nearby_armed_stickies++;
    //     if (ent->is_crit_projectile()) total_danger_score += 100;
    //   }
    // }

    else if (strcmp(class_name, "CTFGrenadePipebombProjectile") == 0) {
      Vec3 velocity = ent->get_velocity();
      bool is_flying_fast = (velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z) > 10000.0f;

      if (is_flying_fast) {
        if (IsProjectileOnCollisionCourse(patient, ent->get_origin(), velocity, 1.0f, MAX_PROJECTILE_PREDICTION_TIME, splash_radius)) {
        total_danger_score += ent->is_crit_projectile() ? 100 : 40;
      }}
      else {
        if (dist_sqr(patient_pos, ent->get_origin()) < splash_radius_sq) {
        total_danger_score += patient->get_health() < 125 ? 60 : 35; 
        if (ent->is_crit_projectile()) total_danger_score += 65;
        }
      }
    }
  }

  if (nearby_armed_stickies > 0) {
    total_danger_score += 10 * nearby_armed_stickies * nearby_armed_stickies + 10;
  }
  
  if (total_danger_score >= 90) return 2;
  if (total_danger_score >= 40) return (patient->get_health() < patient->get_max_health() * 0.7f) ? 2 : 1;
  if (total_danger_score > 0) return 1;

  return 0;
}

int ideal_resistance = 0;
int change_stage = 0;
int change_ticks = 0;

void set_resistance(int resistance, Weapon* medigun) { 
  resistance = std::min(std::max(resistance, 0), 2);
  ideal_resistance = resistance;
  
  int current_res = medigun->get_charge_resist_type();
  if (resistance == current_res) {
    change_stage = 0; 
    return;
  }
  
  if (resistance > current_res) {
    change_stage = resistance - current_res;
  } 
  else {
    change_stage = 3 - current_res + resistance;
  }
}
 
void do_resist_switching(user_cmd* cmd, Weapon* medigun) {
  if (!change_stage) return;
  
  if (medigun->get_charge_resist_type() == ideal_resistance) {
    change_stage = 0;
    change_ticks = 0;
    return;
  }
  
  if (change_ticks <= 0) {
    cmd->buttons |= IN_RELOAD;
    change_stage--;
    change_ticks = 8; 
  } else {
    change_ticks--;
  }

}

struct DangerLevels {
  int bullet = 0;
  int blast = 0;
  int fire = 0;
};

DangerLevels CalculateDangerForPlayer(Player* player) {
  if (!player || player->get_health() <= 0) {return {};}
  return {
  BulletDangerValue(player),
  BlastDangerValue(player),
  FireDangerValue(player)
  };
}

void autovaccinator(user_cmd* cmd) {
  if (config.misc.automatization.autovaccinator == false) return;
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr || localplayer->get_health() <= 0 || localplayer->get_tf_class() != CLASS_MEDIC) return;

  Weapon* weapon = localplayer->get_weapon();
  if (!weapon || weapon->get_def_id() != 998) return;

  DangerLevels self_danger = CalculateDangerForPlayer(localplayer);
  DangerLevels patient_danger = CalculateDangerForPlayer(localplayer->get_healing_target());

  int max_bullet_danger = std::max(self_danger.bullet, patient_danger.bullet);
  int max_blast_danger = std::max(self_danger.blast, patient_danger.blast);
  int max_fire_danger = std::max(self_danger.fire, patient_danger.fire);

  struct DangerSource {
  int level;
  int resistance_type; 
  };

  std::vector<DangerSource> dangers = {
  { max_blast_danger, 1 },
  { max_fire_danger, 2 },
  { max_bullet_danger, 0 }
  };

  auto highest_danger_it = std::max_element(dangers.begin(), dangers.end(), 
  [](const DangerSource& a, const DangerSource& b) {
    return a.level < b.level;
  });

  const auto& highest_danger = *highest_danger_it;
  
  int optimal_res = 0;
  bool should_pop_uber = false;

  if (highest_danger.level > 0) {
    optimal_res = highest_danger.resistance_type;
    if (highest_danger.level > 1) {
      should_pop_uber = true;
    }
  }

  if (weapon->get_charge_resist_type() != optimal_res) {
    set_resistance(optimal_res, weapon);
  }

  do_resist_switching(cmd, weapon);
  
if (should_pop_uber && weapon->get_charge_level() >= 0.25f) {
    if (!localplayer->is_vaccinator_uber_active()) {
        cmd->buttons |= IN_ATTACK2;
    }
}

}

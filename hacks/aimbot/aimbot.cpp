#include "aimbot.hpp"

#include <cmath>

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/engine.hpp"
#include "../../interfaces/engine_trace.hpp"
#include "../../interfaces/convar_system.hpp"

#include "../../gui/config.hpp"
#include "../../classes/player.hpp"

#include "../../print.hpp"

bool is_player_visible(Player *localplayer, Player *entity, int bone) {
    Vec3 target_pos = entity->get_bone_pos(bone);
    Vec3 start_pos = localplayer->get_shoot_pos();

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

void movement_fix(user_cmd *user_cmd, Vec3 original_view_angle, float original_forward_move, float original_side_move) {
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

    user_cmd->forwardmove = cos((yaw_delta) * (M_PI / 180)) * original_forward_move + cos(
                                (yaw_delta + 90.f) * (M_PI / 180)) * original_side_move;
    user_cmd->sidemove = sin((yaw_delta) * (M_PI / 180)) * original_forward_move + sin(
                             (yaw_delta + 90.f) * (M_PI / 180)) * original_side_move;
}

void aimbot(user_cmd *user_cmd) {
    if (!config.aimbot.master) {
        target_player = nullptr;
        return;
    }

    Player *localplayer = entity_list->get_localplayer();
    if (localplayer->get_lifestate() != 1) {
        target_player = nullptr;
        return;
    }

    Weapon *weapon = localplayer->get_weapon();
    if (weapon == nullptr) {
        target_player = nullptr;
        return;
    }

    if (target_player && target_player->is_friend() && config.aimbot.ignore_friends) {
        target_player = nullptr;
    }

    Vec3 original_view_angle = user_cmd->view_angles;
    float original_side_move = user_cmd->sidemove;
    float original_forward_move = user_cmd->forwardmove;

    bool friendlyfire = false;
    static Convar *friendlyfirevar = convar_system->find_var("mp_friendlyfire");
    if (friendlyfirevar) {
        if (friendlyfirevar->get_int() != 0) {
            friendlyfire = true;
        }
    }

    float smallest_fov_angle = __FLT_MAX__;

    for (unsigned int i = 1; i <= entity_list->get_max_entities(); ++i) {
        Player *player = entity_list->player_from_index(i);

        if (player == nullptr ||
            player == localplayer ||
            player->is_dormant() ||
            (player->get_team() == localplayer->get_team() && !friendlyfire) ||
            player->get_lifestate() != 1 ||
            player->is_invulnerable() ||
            (config.aimbot.ignore_friends && player->is_friend())) {
            continue;
        }

        int bone = player->get_tf_class() == CLASS_ENGINEER ? 5 : 2; // Aim at body by default

        // Aim for head
        if (localplayer->get_tf_class() == CLASS_SNIPER) {
            if (localplayer->is_scoped() && player->get_health() > 50)
                bone = player->get_head_bone();
        } else if (localplayer->get_tf_class() == CLASS_SPY) {
            if (weapon->is_headshot_weapon())
                bone = player->get_head_bone();
        }

        Vec3 diff = {
            player->get_bone_pos(bone).x - localplayer->get_shoot_pos().x,
            player->get_bone_pos(bone).y - localplayer->get_shoot_pos().y,
            player->get_bone_pos(bone).z - localplayer->get_shoot_pos().z
        };

        float yaw_hyp = sqrt((diff.x * diff.x) + (diff.y * diff.y));

        float pitch_angle = atan2(diff.z, yaw_hyp) * 180 / M_PI;
        float yaw_angle = atan2(diff.y, diff.x) * 180 / M_PI;

        Vec3 view_angles = {
            .x = -pitch_angle,
            .y = yaw_angle,
            .z = 0
        };

        float x_diff = view_angles.x - original_view_angle.x;
        float y_diff = view_angles.y - original_view_angle.y;

        float x = remainderf(x_diff, 360.0f);
        float y = remainderf(y_diff, 360.0f);

        float clamped_x = x > 89.0f ? 89.0f : x < -89.0f ? -89.0f : x;
        float clamped_y = y > 180.0f ? 180.0f : y < -180.0f ? -180.0f : y;

        float fov = hypotf(clamped_x, clamped_y);

        bool visible = is_player_visible(localplayer, player, bone);

        if (visible && fov <= config.aimbot.fov && fov < smallest_fov_angle) {
            target_player = player;
            smallest_fov_angle = fov;
        }

        if (target_player == player && (fov > config.aimbot.fov || !visible))
            target_player = nullptr;


        if (((is_button_down(config.aimbot.key) && config.aimbot.use_key) || !config.aimbot.use_key) && config.aimbot.
            auto_shoot && target_player == player && localplayer->can_shoot(target_player)) {
            if (config.aimbot.auto_scope && localplayer->get_tf_class() == CLASS_SNIPER && !localplayer->
                is_scoped() && weapon->can_primary_attack())
                user_cmd->buttons |= IN_ATTACK2;

            user_cmd->buttons |= IN_ATTACK;
        }


        if (((is_button_down(config.aimbot.key) && config.aimbot.use_key) || !config.aimbot.use_key) && weapon->
            can_primary_attack() && target_player == player)
            user_cmd->view_angles = view_angles;
    }

    movement_fix(user_cmd, original_view_angle, original_forward_move, original_side_move);
}

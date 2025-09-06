#include "../../gui/config.hpp"

#include "../../interfaces/client.hpp"
#include "../../interfaces/entity_list.hpp"

#include "../../classes/player.hpp"

//#include "../../math.hpp"

void bhop(user_cmd *user_cmd) {
    if (!config.misc.bhop) return;

    Player *localplayer = entity_list->get_localplayer();
    if (!localplayer) return;

    static bool static_jump = false, static_grounded = false, last_attempted = false;
    const bool last_jump = static_jump, last_grounded = static_grounded;
    const bool cur_jump = static_jump = user_cmd->buttons & IN_JUMP, cur_grounded =
            static_grounded = localplayer->get_ground_entity();

    if (cur_jump && last_jump && (cur_grounded ? !localplayer->is_ducking() : true)) {
        if (!(cur_grounded && !last_grounded))
            user_cmd->buttons &= ~IN_JUMP;

        if (!(user_cmd->buttons & IN_JUMP) && cur_grounded && !last_attempted)
            user_cmd->buttons |= IN_JUMP;
    }
    last_attempted = user_cmd->buttons & IN_JUMP;
}
/*
void autostrafe(user_cmd *user_cmd) {
    Player *localplayer = entity_list->get_localplayer();
    if (!localplayer) return;

    if (!(user_cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
        return;

    const float flForward = user_cmd->forwardmove, flSide = user_cmd->sidemove;

    Vec3 vForward, vRight; angle_vectors(user_cmd->view_angles, &vForward, &vRight, nullptr);
    vForward.normalize2d(), vRight.normalize2d();

    const Vec3 vWishDir = vector_angles({ vForward.x * flForward + vRight.x * flSide, vForward.y * flForward + vRight.y * flSide, 0.f });
    const Vec3 vCurDir = vector_angles(localplayer->velocity());
    const float flDirDelta = normalize_angle(vWishDir.y - vCurDir.y);
    if (fabsf(flDirDelta) > 180.f)
        return;;

    const float flTurnScale = remap_val(0.7f, 0.f, 1.f, 0.9f, 1.f);
    const float flRotation = DEG2RAD((flDirDelta > 0.f ? -90.f : 90.f) + flDirDelta * flTurnScale);
    const float flCosRot = cosf(flRotation), flSinRot = sinf(flRotation);

    user_cmd->forwardmove = flCosRot * flForward - flSinRot * flSide;
    user_cmd->sidemove = flSinRot * flForward + flCosRot * flSide;
}*/

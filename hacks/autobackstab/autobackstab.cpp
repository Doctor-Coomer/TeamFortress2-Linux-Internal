#include "../../gui/config.hpp"
#include "../../interfaces/engine.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../classes/player.hpp"
#include "../../classes/weapon.hpp"
#include "../../math.hpp"
#include "../../vec.hpp"

static bool is_weapon_a_knife(Weapon* weapon) {
    if (weapon == nullptr) return false;
    switch (weapon->get_def_id()) {
        case Spy_t_Knife: case Spy_t_KnifeR: case Spy_t_YourEternalReward:
        case Spy_t_ConniversKunai: case Spy_t_TheBigEarner: case Spy_t_TheWangaPrick:
        case Spy_t_TheSharpDresser: case Spy_t_TheSpycicle: case Spy_t_FestiveKnife:
        case Spy_t_TheBlackRose: case Spy_t_SilverBotkillerKnifeMkI: case Spy_t_GoldBotkillerKnifeMkI:
        case Spy_t_RustBotkillerKnifeMkI: case Spy_t_BloodBotkillerKnifeMkI: case Spy_t_CarbonadoBotkillerKnifeMkI:
        case Spy_t_DiamondBotkillerKnifeMkI: case Spy_t_SilverBotkillerKnifeMkII: case Spy_t_GoldBotkillerKnifeMkII:
        case Spy_t_Boneyard: case Spy_t_BlueMew: case Spy_t_BrainCandy:
        case Spy_t_StabbedtoHell: case Spy_t_DressedtoKill: case Spy_t_TopShelf:
        case Spy_t_Blitzkrieg: case Spy_t_Airwolf: case Spy_t_PrinnyMachete:
            return true;
        default:
            return false;
    }
}

void autobackstab(user_cmd* user_cmd) {
    if (!config.misc.automatization.autobackstab) return;

    Player* localplayer = entity_list->get_localplayer();
    if (localplayer == nullptr || localplayer->is_dead()) return;

    if (localplayer->get_tf_class() != CLASS_SPY) return;

    Weapon* active_weapon = localplayer->get_weapon();
    if (!is_weapon_a_knife(active_weapon)) return;

    if (!active_weapon->can_primary_attack()) return;

    if (active_weapon->is_ready_to_backstab()) {
        user_cmd->buttons |= IN_ATTACK;
    }
}
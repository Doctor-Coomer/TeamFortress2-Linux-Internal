#include "../interfaces/client.hpp"
#include "../interfaces/engine.hpp"
#include "../interfaces/entity_list.hpp"
#include "../interfaces/convar_system.hpp"
/*#include "../interfaces/prediction.hpp"
#include "../interfaces/move_helper.hpp"
#include "../interfaces/game_movement.hpp"*/

#include "../gui/config.hpp"

#include "../classes/player.hpp"

//#include "../print.hpp"

#include "../hacks/aimbot/aimbot.cpp"
#include "../hacks/bhop/bhop.cpp"
#include "../hacks/engine_prediction/engine_prediction.cpp"

bool (*client_mode_create_move_original)(void *, float, user_cmd *);

bool client_mode_create_move_hook(void *me, float sample_time, user_cmd *user_cmd) {
    bool rc = client_mode_create_move_original(me, sample_time, user_cmd);

    if (!user_cmd || user_cmd->command_number == 0) {
        return rc;
    }

    if (!engine->is_in_game()) {
        return rc;
    }

    Player *localplayer = entity_list->get_localplayer();

    if (!localplayer || localplayer->get_lifestate() != 1) {
        return rc;
    }

    if (user_cmd->tick_count > 1) {
        bhop(user_cmd);

        start_engine_prediction(user_cmd);
        {
            aimbot(user_cmd);
        }
        end_engine_prediction();

        //no push
        if (Convar *nopush = convar_system->find_var("tf_avoidteammates_pushaway")) {
            if (config.misc.no_push) {
                if (nopush->get_int() != 0)
                    nopush->set_int(0);
            } else {
                if (nopush->get_int() != 1)
                    nopush->set_int(1);
            }
        }
    }

    if (config.aimbot.silent)
        return false;
    else
        return rc;
}

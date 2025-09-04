#include "../interfaces/entity_list.hpp"
#include "../interfaces/input.hpp"

#include "../classes/player.hpp"

#include "../print.hpp"

bool (*should_draw_this_player_original)(void *);

bool should_draw_this_player_hook(void *me) {
    //if (me == entity_list->get_localplayer()) return true;

    return should_draw_this_player_original(me);
}

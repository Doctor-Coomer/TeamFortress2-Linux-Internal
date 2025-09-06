/*#include "../interfaces/entity_list.hpp"

#include "../interfaces/input.hpp"

#include "../classes/player.hpp"*/

bool (*should_draw_local_player_original)(void *);

bool should_draw_local_player_hook(void *me) {
    /*Player *localplayer = entity_list->get_localplayer();

    //return true;

    if (localplayer && me == localplayer)
      return true;
    else
    */
    return should_draw_local_player_original(me);
}

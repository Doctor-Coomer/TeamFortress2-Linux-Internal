/*#include "../interfaces/input.hpp"
#include "../interfaces/prediction.hpp"
#include "../interfaces/client_state.hpp"

#include "../print.hpp"*/

void (*client_create_move_original)(void *, int, float, bool);

void client_create_move_hook(void *me, int sequence_number, float input_sample_frametime, bool active) {
    client_create_move_original(me, sequence_number, input_sample_frametime, active);

    /*
    user_cmd* user_cmd = input->get_user_cmd(sequence_number);
    if (!user_cmd) {
      print("user_cmd == nullptr\n");
      return;
    }
    */

    /*
    prediction->update(client_state->m_nDeltaTick, client_state->m_nDeltaTick > 0, client_state->last_command_ack,
               client_state->lastoutgoingcommand + client_state->chokedcommands);
    */

    //print("%p - %d - %d - %d - %f - %d\n", user_cmd, user_cmd->tick_count, sequence_number, sequence_number%90, input_sample_frametime, active);

    //user_cmd->random_seed = MD5_PseudoRandom(user_cmd->command_number) & 0x7FFFFFFF;

    //bhop(user_cmd);

    //print("%d\n", user_cmd->buttons);

    //print("client create move hooked!\n");
}

#include "cl_move.hpp"

#include "../hacks/tickbase/tickbase.hpp"

#include "../imgui/dearimgui.hpp"

#include "../gui/config.hpp"

#include "../print.hpp"

// Called 66 times a second by the game
void cl_move_hook(float accumulated_extra_samples, bool final_tick) {
  //print("ClMove: %d\n", shifted_ticks);

  shift_all_choked_ticks(accumulated_extra_samples);

  cl_move_original(accumulated_extra_samples, final_tick);      
}
 

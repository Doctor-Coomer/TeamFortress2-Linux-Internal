#include <unistd.h>

#include "../print.hpp"

void (*cl_move_original)(float, bool) = NULL;

// Called 66 times a second by the game
void cl_move_hook(float accumulated_extra_samples, bool final_tick) {
  cl_move_original(accumulated_extra_samples, final_tick);  
}

#ifndef TICKBASE_HPP
#define TICKBASE_HPP

#include "../../imgui/dearimgui.hpp"

#include "../../gui/config.hpp"
#include "../../gui/menu.hpp"

#include "../../hooks/cl_move.hpp"

static bool choke_is_reached = false;
static int choked_ticks = 0;

static void choke_current_tick(void) {
  if (config.misc.exploits.tickbase == false) return;

  // 8dcc is awesome. Go check out everything else they have made and shown.
  // https://8dcc.github.io/reversing/reversing-tf2-bsendpacket.html
  void* current_frame_address = __builtin_frame_address(1);
  void* current_stack_address = (void*)((unsigned long)current_frame_address + 0x8);
  bool* send_packet = (bool*)((unsigned long)current_stack_address + 0xF8);

  if (is_button_down(config.misc.exploits.recharge_button) && choked_ticks < 24 && choke_is_reached == false) {
    *send_packet = false;
    choked_ticks++;
  }

  if (choke_is_reached == true && choked_ticks == 0) {
    choke_is_reached = false;
  }  
}

static void shift_all_choked_ticks(float accumulated_extra_samples) {
  if (config.misc.exploits.tickbase == false) return;
  
  if (menu_focused == false && ImGui::IsImGuiFullyInitialized() &&
      is_button_down(config.aimbot.key) ||
      (ImGui::IsImGuiFullyInitialized() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) &&
      choked_ticks == 24) { // We're not actually shifting all 24 ticks. We will be rebuilding CL_Move eventually

    choke_is_reached = true;
    while (choked_ticks) {
      choked_ticks--;
      cl_move_original(accumulated_extra_samples, choked_ticks == 0);
    }
    
  }

}

#endif

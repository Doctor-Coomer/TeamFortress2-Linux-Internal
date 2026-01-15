#ifndef INDICATORS_HPP
#define INDICATORS_HPP

#include "config.hpp"

#include "../imgui/dearimgui.hpp"

#include "../hacks/tickbase/tickbase.hpp"

static void draw_tickbase_indicator(void) {
  if (config.misc.exploits.tickbase_indicator == false || config.misc.exploits.tickbase == false) return;
  
  ImGuiStyle* style = &ImGui::GetStyle();
  style->WindowTitleAlign = ImVec2(0.5, 0.5);

  ImGui::SetNextWindowSize(ImVec2(200, 50));
  if (ImGui::Begin("Ticks", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
    int tmp = choked_ticks;
    int orig_x = orig_style.FramePadding.x;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(orig_x, 1));
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 16);
    ImGui::SliderInt("##Ticks", &tmp, 0, 24, "%d", ImGuiSliderFlags_NoInput); // Make this non-interactive some how
    if (ImGui::IsItemHovered())
      ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
    ImGui::PopStyleVar(1);
    
  }

  ImGui::End();
}

#endif

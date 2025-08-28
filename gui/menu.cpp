#include "config.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "imgui/dearimgui.hpp"

#include <SDL2/SDL_mouse.h>

inline static bool menu_focused = false;

void get_input(SDL_Event* event) {
  ImGui::KeybindEvent(event, &config.aimbot.key.waiting, &config.aimbot.key.button);
  ImGui::KeybindEvent(event, &config.visuals.thirdperson.key.waiting, &config.visuals.thirdperson.key.button);
}

void draw_aim_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);
  
  ImGui::Checkbox("Master", &config.aimbot.master);


  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();


  ImGui::BeginGroup();

  ImGui::Checkbox("Auto Shoot", &config.aimbot.auto_shoot);  

  ImGui::Text("Aimbot botton: ");
  ImGui::SameLine();
  ImGui::KeybindBox(&config.aimbot.key.waiting, &config.aimbot.key.button);
  ImGui::SameLine();
  ImGui::Checkbox("Use Button", &config.aimbot.use_key);

  ImGui::Checkbox("Silent", &config.aimbot.silent);
  
  ImGui::Text("FOV: ");
  ImGui::SameLine();
  ImGui::SliderFloatHeightPad(" ", &config.aimbot.fov, 0.1f, 180.0f, 1, "%.0f\xC2\xB0");
  
  ImGui::Checkbox("Draw FOV", &config.aimbot.draw_fov);

  ImGui::Checkbox("Sniper auto scope", &config.aimbot.auto_scope);
  
  ImGui::Checkbox("Ignore Friends", &config.aimbot.ignore_friends);
  
  ImGui::EndGroup();  
}

void draw_esp_tab() {  
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);

  ImGui::Checkbox("Master", &config.esp.master);

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* ESP */
  ImGui::BeginGroup();  

  //Player
  ImGui::BeginGroup();
  ImGui::Text("Player");
  ImGui::ColorEdit4("Enemy Color##Player", config.esp.player.enemy_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::ColorEdit4("Team Color##Player", config.esp.player.team_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::ColorEdit4("Friend Color##Player", config.esp.player.friend_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::Checkbox("Box##Player", &config.esp.player.box);
  ImGui::Checkbox("Health Bar##Player", &config.esp.player.health_bar);
  ImGui::SetNextItemWidth(60.0f);
  ImGui::Combo("Position##Player", &config.esp.player.hb_pos, config.esp.player.hb_pos_items, IM_ARRAYSIZE(config.esp.player.hb_pos_items));
  ImGui::Checkbox("Name##Player", &config.esp.player.name);
  ImGui::NewLine();
  ImGui::Text("Flags");
  ImGui::Checkbox("Target##Player", &config.esp.player.flags.target_indicator);
  ImGui::Checkbox("Friend##Player", &config.esp.player.flags.friend_indicator);
  ImGui::SetNextItemWidth(60.0f);
  ImGui::Combo("Position##Player##Flags", &config.esp.player.flags.pos, config.esp.player.flags.pos_items, IM_ARRAYSIZE(config.esp.player.flags.pos_items));
  ImGui::NewLine();
  ImGui::Text("Misc");
  ImGui::Checkbox("Friends##Player", &config.esp.player.friends);
  ImGui::Checkbox("Team##Player", &config.esp.player.team);
  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  //Pickups (health and ammo etc)
  ImGui::BeginGroup();
  ImGui::Text("Pickup");
  ImGui::Checkbox("Box##Pickup", &config.esp.pickup.box);
  ImGui::Checkbox("Name##Pickup", &config.esp.pickup.name);
  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();
  
  //Buildings
  ImGui::BeginGroup();
  ImGui::Text("Buildings");
  ImGui::Checkbox("Box##Buildings", &config.esp.buildings.box);
  ImGui::Checkbox("Health Bar##Buildings", &config.esp.buildings.health_bar);
  ImGui::Checkbox("Name##Buildings", &config.esp.buildings.name);
  ImGui::NewLine();
  ImGui::Text("Misc");
  ImGui::Checkbox("Team##Buildings", &config.esp.buildings.team);
  ImGui::EndGroup();
  
  ImGui::EndGroup();
}

void draw_visuals_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Visuals */
  ImGui::BeginGroup();

  /* Removals */ //maybe make me a drop down
  ImGui::BeginGroup();
  ImGui::Text("Removals");
  ImGui::Checkbox("Scope", &config.visuals.removals.scope);
  ImGui::Checkbox("Zoom", &config.visuals.removals.zoom);

  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::NewLine();
  
  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Camera */
  ImGui::BeginGroup();
  ImGui::Text("Camera");
  ImGui::Text("Key: "); ImGui::SameLine();
  ImGui::KeybindBox(&config.visuals.thirdperson.key.waiting, &config.visuals.thirdperson.key.button); ImGui::SameLine();
  ImGui::Checkbox("Thirdperson", &config.visuals.thirdperson.enabled);
  ImGui::SliderFloatHeightPad("Z##CameraZ", &config.visuals.thirdperson.z, 20.0f, 500.0f, 0, "%.1f");
  ImGui::SliderFloatHeightPad("Y##CameraY",   &config.visuals.thirdperson.y,  -50.0f,  50.0f, 0, "%.1f");
  ImGui::SliderFloatHeightPad("X##CameraX",   &config.visuals.thirdperson.x,  -50.0f,  50.0f, 0, "%.1f");
  
  
  ImGui::NewLine();
  ImGui::NewLine();
  
  ImGui::Checkbox("Override FOV", &config.visuals.override_fov);
  ImGui::SliderFloat(" ", &config.visuals.custom_fov, 30.1f, 150.0f, "%.0f\xC2\xB0");
  ImGui::EndGroup();
  
  ImGui::EndGroup();
}

void draw_misc_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  ImGui::BeginGroup();
  
  ImGui::Text("General");
  ImGui::Checkbox("Bhop", &config.misc.bhop);
  ImGui::Checkbox("Bypass sv_pure", &config.misc.bypasspure);
  ImGui::Checkbox("No Push", &config.misc.no_push);  

  ImGui::EndGroup();
}

void draw_debug_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();
  
  ImGui::BeginGroup();

  ImGui::SliderInt("Font Height", &config.debug.font_height, 6, 60);  
  ImGui::SliderInt("Font Weight", &config.debug.font_weight, 50, 800);  
  ImGui::Checkbox("Draw All Entities", &config.debug.debug_render_all_entities);
  
  ImGui::EndGroup();
}

void draw_tab(ImGuiStyle* style, const char* name, int* tab, int index) {
  ImVec4 orig_box_color = ImVec4(0.15, 0.15, 0.15, 1);
  
  if (*tab == index) {
    style->Colors[ImGuiCol_Button] = ImVec4(orig_box_color.x + 0.15, orig_box_color.y + 0.15, orig_box_color.z + 0.15, 1.00f);
  } else {
    style->Colors[ImGuiCol_Button] = ImVec4(0.15, 0.15, 0.15, 1);
  }
  
  if (ImGui::Button(name, ImVec2(80, 30))) {
    *tab = index;
  }
  style->Colors[ImGuiCol_Button] = ImVec4(0.15, 0.15, 0.15, 1);
}

void draw_menu() {
  ImGui::SetNextWindowSize(ImVec2(600, 350));
  if (ImGui::Begin("Team Fortress 2 GNU/Linux", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    static int tab = 0;

    ImGuiStyle* style = &ImGui::GetStyle();

    style->Colors[ImGuiCol_WindowBg]         = ImVec4(0.1, 0.1, 0.1, 1);
    style->Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.05, 0.05, 0.05, 1);
    style->Colors[ImGuiCol_TitleBg]          = ImVec4(0.05, 0.05, 0.05, 1);
    style->Colors[ImGuiCol_CheckMark]        = ImVec4(0.869346734, 0.450980392, 0.211764706, 1);
    style->Colors[ImGuiCol_FrameBg]          = ImVec4(0.15, 0.15, 0.15, 1);
    style->Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
    style->Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);
    style->Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
    style->Colors[ImGuiCol_ButtonActive]     = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);
    style->Colors[ImGuiCol_SliderGrab]       = ImVec4(0.869346734, 0.450980392, 0.211764706, 1);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.899346734, 0.480980392, 0.241764706, 1);
    style->GrabMinSize = 2;
    
    ImGui::BeginGroup();
    draw_tab(style, "Aimbot", &tab, 0);
    draw_tab(style, "ESP", &tab, 1);
    draw_tab(style, "Visuals", &tab, 2);
    draw_tab(style, "Misc", &tab, 3);
    draw_tab(style, "Debug", &tab, 4);

    switch (tab) {
    case 0:
      draw_aim_tab();
      break;
    case 1:
      draw_esp_tab();
      break;      
    case 2:
      draw_visuals_tab();
      break;
    case 3:
      draw_misc_tab();
      break;
    case 4:
      draw_debug_tab();
      break;      
    }
  }
  
  ImGui::End();  
}


#ifndef MENU_HPP
#define MENU_HPP

#include "config.hpp"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "../imgui/dearimgui.hpp"

#include <SDL2/SDL_mouse.h>

#include "../hacks/navbot/navmesh.hpp"

inline static SDL_Window* sdl_window = NULL;
inline static bool menu_focused = false;

inline static bool did_render_pass = false;

static void get_input(SDL_Event* event) {
  ImGui::KeybindEvent(event, &config.aimbot.key.waiting, &config.aimbot.key.button);
  ImGui::KeybindEvent(event, &config.visuals.thirdperson.key.waiting, &config.visuals.thirdperson.key.button);
}

static void draw_watermark() {
  ImGui::SetNextWindowPos(ImVec2(10, 10)); 
  ImGui::SetNextWindowSize(ImVec2(150, 30));
  ImGui::Begin("##Watermark", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    
  ImGui::TextCentered("I Use Arch BTW!!!");
  ImGui::End();
}

static void draw_aim_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);
  
  ImGui::Checkbox("Master", &config.aimbot.master);

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();


  ImGui::BeginChild("##AimbotMasterChild");

  ImGui::BeginGroup();
  ImGui::Text("General");
  ImGui::Checkbox("Auto Shoot", &config.aimbot.auto_shoot);  

  const char* target_items[] = { "FOV", "Distance", "Least Health", "Most Health" };
  ImGui::Text("Target: ");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::Combo("##TargetType", (int*)&config.aimbot.target_type, target_items, IM_ARRAYSIZE(target_items));
  
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
  ImGui::EndGroup();

  ImGui::NewLine();
  ImGui::Separator();

  ImGui::BeginGroup();
  ImGui::Text("Class");
  ImGui::Checkbox("Sniper: Auto Scope", &config.aimbot.auto_scope);
  ImGui::Checkbox("Sniper: Auto Unscope", &config.aimbot.auto_unscope);
  ImGui::Checkbox("Sniper: Scoped Only", &config.aimbot.scoped_only);
  ImGui::EndGroup();
  
  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  ImGui::BeginGroup();
  ImGui::Text("Misc");
  ImGui::Checkbox("Ignore Friends", &config.aimbot.ignore_friends);
  ImGui::EndGroup();
  
  ImGui::EndChild();
}

static void draw_esp_tab() {  
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);

  ImGui::Checkbox("Master", &config.esp.master);

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* ESP */
  ImGui::BeginChild("##EspMasterChild");

  //Player
  ImGui::BeginGroup();
  ImGui::Text("Player");
  ImGui::ColorEdit4("Enemy Color##Player", config.esp.player.enemy_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::ColorEdit4("Team Color##Player", config.esp.player.team_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::ColorEdit4("Friend Color##Player", config.esp.player.friend_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::Checkbox("Box##Player", &config.esp.player.box);
  ImGui::Checkbox("Health Bar##Player", &config.esp.player.health_bar);
  ImGui::Checkbox("Name##Player", &config.esp.player.name);
  ImGui::NewLine();
  ImGui::Text("Flags");
  ImGui::Checkbox("Target##Player", &config.esp.player.flags.target_indicator);
  ImGui::Checkbox("Friend##Player", &config.esp.player.flags.friend_indicator);
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

  ImGui::NewLine();
  
  //Objectives
  ImGui::Text("Intelligence");
  ImGui::Checkbox("Box##Intelligence", &config.esp.intelligence.box);
  ImGui::Checkbox("Name##Intelligence", &config.esp.intelligence.name);  
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
  
  ImGui::EndChild();
}

static void draw_visuals_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Visuals */
  ImGui::BeginChild("##VisualsMasterChild");
  
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
  
  ImGui::EndChild();
}

static void draw_misc_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Misc */
  ImGui::BeginChild("##MiscMasterChild");
  
  ImGui::Text("Movement");
  ImGui::Checkbox("Bhop", &config.misc.movement.bhop);
  ImGui::Checkbox("No Push", &config.misc.movement.no_push);

  ImGui::NewLine();
  ImGui::Text("Automatization");
  ImGui::Checkbox("Autovaccinator", &config.misc.automatization.autovaccinator);
  ImGui::Checkbox("Autobackstab", &config.misc.automatization.autobackstab);

  ImGui::NewLine();

  ImGui::Text("Exploits");
  ImGui::Checkbox("Bypass sv_pure", &config.misc.exploits.bypasspure);
  ImGui::Checkbox("No Engine Sleep", &config.misc.exploits.no_engine_sleep);  
  
  ImGui::EndChild();
}

static void draw_navbot_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);
  
  ImGui::Checkbox("Master", &config.navbot.master);

  ImGui::EndGroup();
  
  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();


  ImGui::BeginChild("##NavbotMasterChild");

  ImGui::Text("General");
  ImGui::Checkbox("Walk Path", &config.navbot.walk);

  ImGui::NewLine();

  ImGui::Checkbox("Look At Path", &config.navbot.look_at_path);
  ImGui::SliderFloat("Look Smoothness", &config.navbot.look_smoothness, 0, 100);  

  ImGui::EndChild();
}

static void draw_debug_tab() {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Debug */
  ImGui::BeginChild("##DebugMasterChild");

  ImGui::SliderInt("Font Height", &config.debug.font_height, 6, 60);  
  ImGui::SliderInt("Font Weight", &config.debug.font_weight, 50, 800);  
  ImGui::Checkbox("Draw All Entities", &config.debug.debug_render_all_entities);
  ImGui::Checkbox("Draw Navmesh", &config.debug.debug_draw_navmesh);    
  ImGui::Checkbox("Draw Navbot Path", &config.debug.debug_draw_navbot_path);
  ImGui::Checkbox("Draw Navmesh Current Area", &config.debug.debug_draw_navbot_current_area);  
  ImGui::Checkbox("Draw Navmesh Next Area", &config.debug.debug_draw_navbot_next_area);  
  ImGui::Checkbox("Draw Navbot Goal", &config.debug.debug_draw_navbot_goal);  
  if(ImGui::Button("Force reparse navmesh")) {
    mesh.map_name = "";
  }
  
  ImGui::EndChild();
}

static void draw_tab(ImGuiStyle* style, const char* name, int* tab, int index) {
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

static void draw_menu() {
  ImGui::SetNextWindowSize(ImVec2(600, 350));
  if (ImGui::Begin("Team Fortress 2 GNU/Linux", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {

    
    ImGuiStyle* style = &ImGui::GetStyle();
    
    static int tab = 0;
    ImGui::BeginGroup();
    draw_tab(style, "Aimbot", &tab, 0);
    draw_tab(style, "ESP", &tab, 1);
    draw_tab(style, "Visuals", &tab, 2);
    draw_tab(style, "Misc", &tab, 3);
    draw_tab(style, "Navbot", &tab, 4);
    draw_tab(style, "Debug", &tab, 5);

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
      draw_navbot_tab();
      break;
    case 5:
      draw_debug_tab();
      break;      
    }
  }
  
  ImGui::End();  
}

#endif

#ifndef MENU_HPP
#define MENU_HPP

#include "config.hpp"

#include "../imgui/dearimgui.hpp"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include <SDL2/SDL_mouse.h>

#include "../hacks/navbot/navmesh.hpp"
#include "../hacks/pishock/pishock.hpp"

inline static SDL_Window* sdl_window = NULL;
inline static bool menu_focused = false;

static void get_input(SDL_Event* event) {
  ImGui::KeybindEvent(event, &config.aimbot.key.waiting, &config.aimbot.key.button);
  ImGui::KeybindEvent(event, &config.visuals.thirdperson.key.waiting, &config.visuals.thirdperson.key.button);
  ImGui::KeybindEvent(event, &config.misc.exploits.recharge_button.waiting, &config.misc.exploits.recharge_button.button);
}

static void set_imgui_theme(void) {
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

  style->Colors[ImGuiCol_Header]           = ImVec4(0.18, 0.18, 0.18, 1);
  style->Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
  style->Colors[ImGuiCol_HeaderActive]     = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

  style->ScrollbarRounding = 0;
  style->ScrollbarSize = 2;
}

static void draw_watermark(void) {
  if (config.misc.menu.enabled == false) return;

  ImGui::SetNextWindowPos(ImVec2(10, 10)); 
  ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(config.misc.menu.text.c_str()).x + 31, 30));
  ImGui::Begin("##Watermark", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);  
  ImGui::TextCentered(config.misc.menu.text);
  ImGui::End();
}

static void draw_aim_tab(void) {
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

static void draw_esp_tab(void) {  
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);

  ImGui::Checkbox("Master", &config.esp.master);

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* ESP */
  ImGui::BeginChild("##EspMasterChild");

  //Player
  ImGui::BeginChild("##EspPlayerChild", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
  ImGui::Text("Player");
  ImGui::Separator();
  ImGui::ColorEdit4("Enemy Color##EspPlayer", config.esp.player.enemy_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::ColorEdit4("Team Color##EspPlayer", config.esp.player.team_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::ColorEdit4("Friend Color##EspPlayer", config.esp.player.friend_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::Checkbox("Box##EspPlayer", &config.esp.player.box);
  ImGui::Checkbox("Health Bar##EspPlayer", &config.esp.player.health_bar);
  ImGui::Checkbox("Name##EspPlayer", &config.esp.player.name);
  //ImGui::NewLine();
  if (ImGui::BeginCombo("##Flags", "Flags", ImGuiComboFlags_WidthFitPreview)) {

    {
      ImGui::Checkbox("Target##EspPlayer", &config.esp.player.flags.target_indicator);
      ImGui::Checkbox("Friend##EspPlayer", &config.esp.player.flags.friend_indicator);
      ImGui::Checkbox("Scoped##EspPlayer", &config.esp.player.flags.scoped_indicator);
    }

    ImGui::EndCombo();
  }
  ImGui::NewLine();
  ImGui::Text("Misc");
  ImGui::Checkbox("Enemy##EspPlayer", &config.esp.player.enemy);
  ImGui::Checkbox("Team##EspPlayer", &config.esp.player.team);
  ImGui::Checkbox("Friends##EspPlayer", &config.esp.player.friends);
  ImGui::EndChild();
  
  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  //Pickups (health and ammo etc)
  ImGui::BeginChild("##EspPickupChild", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
  ImGui::Text("Pickup");
  ImGui::Separator();
  ImGui::Checkbox("Box##EspPickup", &config.esp.pickup.box);
  ImGui::Checkbox("Name##EspPickup", &config.esp.pickup.name);

  ImGui::NewLine();
  
  //Objectives
  ImGui::Text("Intelligence");
  ImGui::Separator();
  ImGui::Checkbox("Box##EspIntelligence", &config.esp.intelligence.box);
  ImGui::Checkbox("Name##EspIntelligence", &config.esp.intelligence.name);  
  ImGui::EndChild();
  
  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();
  
  //Buildings
  ImGui::BeginChild("##EspBuildingsChild", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
  ImGui::Text("Buildings");
  ImGui::Separator();
  ImGui::Checkbox("Box##EspBuildings", &config.esp.buildings.box);
  ImGui::Checkbox("Health Bar##EspBuildings", &config.esp.buildings.health_bar);
  ImGui::Checkbox("Name##EspBuildings", &config.esp.buildings.name);
  ImGui::NewLine();
  ImGui::Text("Misc");
  ImGui::Checkbox("Team##EspBuildings", &config.esp.buildings.team);
  ImGui::EndChild();
  
  ImGui::EndChild();
}

static void draw_chams_tab(void) {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 27);
  
  ImGui::Checkbox("Master", &config.chams.master);

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  ImGui::BeginChild("##ChamsMasterChild");

  const char* target_material_types[] = { "None", "Flat", "Shaded", "Fresnel" };
  
  //Player
  ImGui::BeginChild("##EspPlayerChild", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
  ImGui::Text("Player");
  ImGui::Separator();
  ImGui::Checkbox("Enemy##EnemyChamsEnemyPlayer", &config.chams.player.enemy);
  ImGui::ColorEdit4("Enemy Color##ChamsEnemyPlayer", config.chams.player.enemy_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsEnemyPlayerMaterial", (int*)&config.chams.player.enemy_material_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  ImGui::ColorEdit4("Occluded##ChamsEnemyPlayer", config.chams.player.enemy_color_z.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);   
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsEnemyPlayerMaterialz", (int*)&config.chams.player.enemy_material_z_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  if (ImGui::BeginCombo("##ChamsEnemyPlayerFlags", "Enemy Flags", ImGuiComboFlags_WidthFitPreview)) {
    {
      ImGui::Checkbox("Ignore Z", &config.chams.player.enemy_flags.ignore_z);
      ImGui::Checkbox("Wireframe", &config.chams.player.enemy_flags.wireframe);
    }
    ImGui::EndCombo();
  }
  ImGui::ColorEdit4("Overlay Color##OverlayChamsEnemyPlayer", config.chams.player.enemy_overlay_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##OverlayChamsEnemyPlayerMaterial", (int*)&config.chams.player.enemy_overlay_material_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  ImGui::ColorEdit4("Occluded##OverlayChamsEnemyPlayer", config.chams.player.enemy_overlay_color_z.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);   
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##OverlayChamsEnemyPlayerMaterialz", (int*)&config.chams.player.enemy_overlay_material_z_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  if (ImGui::BeginCombo("##OverlayChamsEnemyPlayerFlags", "Overlay Flags", ImGuiComboFlags_WidthFitPreview)) {
    {
      ImGui::Checkbox("Ignore Z", &config.chams.player.enemy_overlay_flags.ignore_z);
      ImGui::Checkbox("Wireframe", &config.chams.player.enemy_overlay_flags.wireframe);
    }
    ImGui::EndCombo();
  }
  ImGui::NewLine();
    
  ImGui::Checkbox("Team##ChamsTeamPlayer", &config.chams.player.team);
  ImGui::ColorEdit4("Team Color##ChamsTeamPlayer", config.chams.player.team_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsTeamPlayerMaterial", (int*)&config.chams.player.team_material_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  ImGui::ColorEdit4("Occluded##ChamsTeamPlayer", config.chams.player.team_color_z.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);  
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsTeamPlayerMaterialz", (int*)&config.chams.player.team_material_z_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  if (ImGui::BeginCombo("##ChamsTeamPlayerFlags", "Team Flags", ImGuiComboFlags_WidthFitPreview)) {
    {
      ImGui::Checkbox("Ignore Z", &config.chams.player.team_flags.ignore_z);
      ImGui::Checkbox("Wireframe", &config.chams.player.team_flags.wireframe);
    }
    ImGui::EndCombo();
  }
  ImGui::NewLine();

  ImGui::Checkbox("Friends##ChamsFriendPlayer", &config.chams.player.friends);
  ImGui::ColorEdit4("Friend Color##ChamsFriendPlayer", config.chams.player.friend_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsFriendPlayerMaterial", (int*)&config.chams.player.friend_material_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  ImGui::ColorEdit4("Occluded##ChamsFriendPlayer", config.chams.player.friend_color_z.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);  
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsFriendPlayerMaterialz", (int*)&config.chams.player.friend_material_z_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  if (ImGui::BeginCombo("##ChamsFriendPlayerFlags", "Friend Flags", ImGuiComboFlags_WidthFitPreview)) {
    {
      ImGui::Checkbox("Ignore Z", &config.chams.player.friends_flags.ignore_z);
      ImGui::Checkbox("Wireframe", &config.chams.player.friends_flags.wireframe);
    }
    ImGui::EndCombo();
  }
  ImGui::NewLine();

  ImGui::Checkbox("Local##ChamsLocalPlayer", &config.chams.player.local);
  ImGui::ColorEdit4("Local Color##ChamsLocalPlayer", config.chams.player.local_color.to_arr(), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
  ImGui::SameLine(); ImGui::SetNextItemWidth(97); ImGui::Combo("##ChamsLocalPlayerMaterial", (int*)&config.chams.player.local_material_type, target_material_types, IM_ARRAYSIZE(target_material_types));
  if (ImGui::BeginCombo("##ChamsLocalPlayerFlags", "Local Flags", ImGuiComboFlags_WidthFitPreview)) {
    {
      ImGui::Checkbox("Wireframe", &config.chams.player.local_flags.wireframe);
    }
    ImGui::EndCombo();
  }
  ImGui::NewLine();
  ImGui::EndChild(); // ##EspPlayerChild
  
  ImGui::EndChild();
}

static void draw_visuals_tab(void) {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Visuals */
  ImGui::BeginChild("##VisualsMasterChild");
  
  /* Removals */ //maybe make me a drop down, eventually:TM:
  ImGui::BeginChild("##VisualsRemovalsChild", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
  if (ImGui::BeginCombo("##Removals", "Removals", ImGuiComboFlags_WidthFitPreview)) {

    {
      ImGui::Checkbox("Scope", &config.visuals.removals.scope);
      ImGui::Checkbox("Zoom", &config.visuals.removals.zoom);    
    }

    ImGui::EndCombo();
  }

  ImGui::EndChild();
  
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
  ImGui::SliderFloat(" ##OverrideFOV", &config.visuals.custom_fov, 30.1f, 150.0f, "%.0f\xC2\xB0");

  ImGui::Checkbox("Override Viewmodel FOV", &config.visuals.override_viewmodel_fov);
  ImGui::SliderFloat(" ##OverrideViewmodelFOV", &config.visuals.custom_viewmodel_fov, 30.1f, 150.0f, "%.0f\xC2\xB0");
  ImGui::EndGroup();
  
  ImGui::EndChild();
}

static void draw_misc_tab(void) {
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

  ImGui::Text("Exploits");
  ImGui::Checkbox("Bypass sv_pure", &config.misc.exploits.bypasspure);
  ImGui::Checkbox("No Engine Sleep", &config.misc.exploits.no_engine_sleep);
  ImGui::NewLine();
  ImGui::Checkbox("Tickbase", &config.misc.exploits.tickbase);
  ImGui::Checkbox("Tickbase indicator", &config.misc.exploits.tickbase_indicator);
  ImGui::Text("Tick choke key:");
  ImGui::SameLine();
  ImGui::KeybindBox(&config.misc.exploits.recharge_button.waiting, &config.misc.exploits.recharge_button.button);
  
  /*
  ImGui::NewLine();

  ImGui::Text("Automation");
  const char* class_items[] = { "Random", "Scout", "Sniper", "Soldier", "Demoman", "Medic", "Heavy", "Pyro", "Spy", "Engineer" };
  ImGui::SetNextItemWidth(120);
  ImGui::Combo("##TargetType", (int*)&config.misc.automation.class_selected, class_items, IM_ARRAYSIZE(class_items));    
  ImGui::SameLine();
  ImGui::Checkbox("Auto Class Select", &config.misc.automation.auto_class_select);
  */

  ImGui::NewLine();

  ImGui::Text("Menu");
  ImGui::Checkbox("Watermark", &config.misc.menu.enabled);
  ImGui::InputText("Watermark Text", &config.misc.menu.text);
  
  ImGui::EndChild();
}

static void draw_navbot_tab(void) {
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

static void draw_debug_tab(void) {
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 21);

  ImGui::Text(" ");

  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  /* Debug */
  ImGui::BeginChild("##DebugMasterChild");

  ImGui::TextColored(ImVec4(255, 0, 0, 255), "Warning: None of these are intended to be actual features.\nThey are for developers.\nIf you find these helpful, then have at it!");
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
  ImGui::Checkbox("Show active flag IDs on players", &config.debug.show_active_flag_ids_of_players);
  ImGui::Checkbox("Disable friend checks", &config.debug.disable_friend_checks);
  
  ImGui::NewLine();
  ImGui::Text("PiShock");
  ImGui::Checkbox("Master", &config.debug.pishock_master);
  ImGui::SameLine();
  ImGui::PushButtonRepeat(false);
  if (ImGui::Button("Send Now")) pishock();
  ImGui::PopButtonRepeat();
  ImGui::Checkbox("Send On Kill", &config.debug.pishock_on_kill);
  ImGui::SameLine(); ImGui::Checkbox("Send On Death", &config.debug.pishock_on_death);
  ImGui::InputText("Username:", &config.debug.username);
  ImGui::InputText("Sharecode:", &config.debug.sharecode);
  ImGui::InputText("Api Key:", &config.debug.apikey);
  ImGui::SliderInt("Intensity:", &config.debug.intensity, 1, 100);
  ImGui::SliderInt("Duration:", &config.debug.duration, 1, 15);
  const char* operation_types[] = { "Shock", "Vibrate", "Beep" };
  ImGui::Combo("Operation", (int*)&config.debug.operation, operation_types, IM_ARRAYSIZE(operation_types));  
  
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

static void draw_menu(void) {
  ImGuiStyle* style = &ImGui::GetStyle();
  style->WindowTitleAlign = ImVec2(0, 0.5);

  ImGui::SetNextWindowSize(ImVec2(600, 350));
  if (ImGui::Begin("Team Fortress 2 GNU/Linux", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        
    static int tab = 0;
    ImGui::BeginGroup();
    draw_tab(style, "Aimbot", &tab, 0);
    draw_tab(style, "ESP", &tab, 1);
    draw_tab(style, "Chams", &tab, 2);
    draw_tab(style, "Visuals", &tab, 3);
    draw_tab(style, "Misc", &tab, 4);
    draw_tab(style, "Navbot", &tab, 5);
    draw_tab(style, "Debug", &tab, 6);

    switch (tab) {
    case 0:
      draw_aim_tab();
      break;
    case 1:
      draw_esp_tab();
      break;      
    case 2:
      draw_chams_tab();
      break;
    case 3:
      draw_visuals_tab();
      break;
    case 4:
      draw_misc_tab();
      break;
    case 5:
      draw_navbot_tab();
      break;
    case 6:
      draw_debug_tab();
      break;      
    }
  }
  
  ImGui::End();  
}

#endif

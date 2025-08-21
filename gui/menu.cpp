#include "config.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "imgui/dearimgui.hpp"
#include "imgui/imgui_stdlib.h"

#include <cstdio>

#include <SDL2/SDL_mouse.h>
#include "../hacks/navmesh/navparser.hpp"
#include "../hacks/navmesh/navengine.hpp"
#include "../interfaces/engine.hpp"

static ImGuiStyle orig_style;

void get_input(SDL_Event* event) {
  ImGui::KeybindEvent(event, &config.aimbot.key.waiting, &config.aimbot.key.button);
  ImGui::KeybindEvent(event, &config.visuals.thirdperson.key.waiting, &config.visuals.thirdperson.key.button);
}

void draw_config_tab() {
  ImGui::BeginGroup();

  static std::string cfg_name;
  static std::vector<std::string> cfg_list;
  static int selected = -1;
  static char status_buf[256] = {0};
  static ImVec4 status_col = ImVec4(0.7f, 0.7f, 0.7f, 1);

  if (cfg_list.empty()) cfg_list = cfgio::List();

  ImGui::SetNextItemWidth(-1);
  ImGui::InputText("##CfgName", &cfg_name);
  ImGui::Spacing();
  if (ImGui::Button("Save")) {
    if (!cfg_name.empty()) {
      if (cfgio::Save(cfg_name)) {
        cfg_list = cfgio::List();
        std::snprintf(status_buf, sizeof(status_buf), "Saved '%s'", cfg_name.c_str());
        status_col = ImVec4(0.4f, 1.0f, 0.4f, 1);
      } else {
        const char* err = cfgio::GetLastError();
        std::snprintf(status_buf, sizeof(status_buf), "Save error: %s", err ? err : "unknown");
        status_col = ImVec4(1.0f, 0.5f, 0.2f, 1);
      }
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    if (!cfg_name.empty()) {
      if (cfgio::Load(cfg_name)) {
        std::snprintf(status_buf, sizeof(status_buf), "Loaded '%s'", cfg_name.c_str());
        status_col = ImVec4(0.4f, 1.0f, 0.4f, 1);
      } else {
        const char* err = cfgio::GetLastError();
        std::snprintf(status_buf, sizeof(status_buf), "Load error: %s", err ? err : "unknown");
        status_col = ImVec4(1.0f, 0.5f, 0.2f, 1);
      }
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) {
    cfg_list = cfgio::List();
    selected = -1;
  }

  if (ImGui::BeginCombo("Existing", (selected >= 0 && selected < (int)cfg_list.size()) ? cfg_list[selected].c_str() : "<none>")) {
    for (int i = 0; i < (int)cfg_list.size(); ++i) {
      const bool is_sel = (selected == i);
      if (ImGui::Selectable(cfg_list[i].c_str(), is_sel)) {
        selected = i;
        cfg_name = cfg_list[i];
      }
      if (is_sel) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  if (status_buf[0]) {
    ImGui::TextColored(status_col, "%s", status_buf);
  }

  ImGui::Separator();
  ImGui::Text("Config files live in /opt/tuxbot");

  ImGui::EndGroup();
}

void draw_aim_tab() {
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
  ImGui::SliderFloat(" ", &config.aimbot.fov, 0.1f, 180.0f, "%.0f\xC2\xB0");
  
  ImGui::Checkbox("Draw FOV", &config.aimbot.draw_fov);

  ImGui::Checkbox("Ignore Friends", &config.aimbot.ignore_friends);

  ImGui::Separator();
  ImGui::Text("Modules");
  ImGui::Checkbox("Hitscan", &config.aimbot.hitscan.enabled);
  ImGui::SameLine();
  // coomer pls add multidropdowns
  ImGui::BeginGroup();
  ImGui::Text("Hitscan Modifiers");
  bool hitscan_scoped_only = (config.aimbot.hitscan.modifiers & Aim::Hitscan::ScopedOnly) != 0;
  if (ImGui::Checkbox("Scoped Only##Hitscan", &hitscan_scoped_only)) {
    if (hitscan_scoped_only) config.aimbot.hitscan.modifiers |= Aim::Hitscan::ScopedOnly;
    else                     config.aimbot.hitscan.modifiers &= ~Aim::Hitscan::ScopedOnly;
  }
  ImGui::SameLine();
  bool hitscan_wait_hs = (config.aimbot.hitscan.modifiers & Aim::Hitscan::WaitForHeadshot) != 0;
  if (ImGui::Checkbox("Wait for Headshot##Hitscan", &hitscan_wait_hs)) {
    if (hitscan_wait_hs) config.aimbot.hitscan.modifiers |= Aim::Hitscan::WaitForHeadshot;
    else                 config.aimbot.hitscan.modifiers &= ~Aim::Hitscan::WaitForHeadshot;
  }
  ImGui::EndGroup();
  ImGui::Checkbox("Projectile", &config.aimbot.projectile.enabled);
  ImGui::Checkbox("Melee", &config.aimbot.melee.enabled);

  const char* hitbox_items[] = { "Chest", "Head" };
  ImGui::Text("Hitscan Hitbox");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::Combo("##HitscanHitbox", &config.aimbot.hitscan.hitbox, hitbox_items, IM_ARRAYSIZE(hitbox_items));

  ImGui::Text("Projectile Hitbox");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::Combo("##ProjectileHitbox", &config.aimbot.projectile.hitbox, hitbox_items, IM_ARRAYSIZE(hitbox_items));
  
  ImGui::EndGroup();  
}

void draw_esp_tab() {  
  /* ESP */
  ImGui::BeginGroup();  

  //Player
  ImGui::BeginGroup();
  ImGui::Text("Player");
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
    
  ImGui::EndGroup();
}

void draw_visuals_tab() {
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
  ImGui::SliderFloat(" ##Camera", &config.visuals.thirdperson.distance, 20.0f, 500.0f, "%.1f");
  
  
  ImGui::NewLine();
  ImGui::NewLine();
  
  ImGui::Checkbox("Override FOV", &config.visuals.override_fov);
  ImGui::SliderFloat(" ", &config.visuals.custom_fov, 30.1f, 150.0f, "%.0f\xC2\xB0");
  ImGui::EndGroup();
  
  ImGui::EndGroup();
}

void draw_misc_tab() {
  ImGui::BeginGroup();

  ImGui::Text("General");
  ImGui::Checkbox("Bhop", &config.misc.bhop);
  ImGui::Checkbox("Bypass sv_pure", &config.misc.bypasspure);
  ImGui::Checkbox("No Push", &config.misc.no_push);  

  ImGui::Separator();
  ImGui::Checkbox("AutoJoin", &config.misc.autojoin);
  ImGui::Text("Force Class");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(150);
  ImGui::Combo("##AutoJoinClass", &config.misc.autojoin_class, TF2_CLASS_DISPLAY_NAMES, IM_ARRAYSIZE(TF2_CLASS_DISPLAY_NAMES));

  ImGui::EndGroup();
}

void draw_nav_tab() {
  ImGui::BeginGroup();
  
  bool in_game = engine && engine->is_in_game();
  
  if (!in_game) {
    ImGui::TextColored(ImVec4(1,0.6f,0.2f,1), "Nav features only available in-game");
    ImGui::Text("Join a match to use navigation features");
    ImGui::EndGroup();
    return;
  }
  

  if (config.nav.master) {
    ImGui::Checkbox("Enable nav engine", &config.nav.engine_enabled);
    ImGui::Checkbox("Navmesh visualizer", &config.nav.visualize_navmesh);
    ImGui::Checkbox("Path visualization", &config.nav.visualize_path);

    ImGui::Checkbox("Navbot", &config.nav.navbot);

    ImGui::Separator();
    ImGui::Text("View");
    ImGui::Checkbox("Look at path", &config.nav.look_at_path);
    ImGui::Checkbox("Smoothed##LookAtPath", &config.nav.look_at_path_smoothed);

    ImGui::Separator();
    ImGui::Text("Tasks");
    ImGui::Checkbox("Snipe enemies", &config.nav.snipe_enemies);
    ImGui::Text("Snipe Preferred Range per Class (HU)");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Scout##SnipeRange", &config.nav.snipe_range.scout, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Soldier##SnipeRange", &config.nav.snipe_range.soldier, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Pyro##SnipeRange", &config.nav.snipe_range.pyro, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Demoman##SnipeRange", &config.nav.snipe_range.demoman, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Heavy##SnipeRange", &config.nav.snipe_range.heavy, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Engineer##SnipeRange", &config.nav.snipe_range.engineer, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Medic##SnipeRange", &config.nav.snipe_range.medic, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Sniper##SnipeRange", &config.nav.snipe_range.sniper, 300.0f, 1800.0f, "%.0f HU");
    ImGui::SetNextItemWidth(220);
    ImGui::SliderFloat("Spy##SnipeRange", &config.nav.snipe_range.spy, 300.0f, 1800.0f, "%.0f HU");
    ImGui::Text("Snipe Planner");
    ImGui::SetNextItemWidth(120);
    ImGui::SliderInt("Repath Ticks##Snipe", &config.nav.snipe_repath_ticks, 1, 32);
    ImGui::SetNextItemWidth(140);
    ImGui::SliderFloat("Replan Move##Snipe", &config.nav.snipe_replan_move_threshold, 16.0f, 256.0f, "%.0f HU");

    if (config.nav.engine_enabled) {
      if (nav::IsLoaded()) {
        const nav::Info &info = nav::GetInfo();
        ImGui::Text("Loaded: %s", info.path.c_str());
        ImGui::Text("Size: %zu bytes", info.size);
        ImGui::Text("Magic: %s (0x%08X)", info.magic_ascii, info.magic_le);
        ImGui::Text("Version: %d", info.version);
      } else {
        const char *err = nav::GetError();
        if (!err) err = nav::GetLastError();
        if (err) {
          ImGui::TextColored(ImVec4(1,0.4f,0.2f,1), "Error: %s", err);
        } else {
          ImGui::Text("No nav loaded.");
        }
      }
    } else {
      ImGui::Text("navengine disabled.");
    }
  } else {
    ImGui::Text("navengine disabled.");
  }

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
    const float sidebar_w = 100.0f;

    if (ImGui::BeginChild("Sidebar", ImVec2(sidebar_w, 0), true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
      draw_tab(style, "Aimbot", &tab, 0);
      draw_tab(style, "ESP", &tab, 1);
      draw_tab(style, "Visuals", &tab, 2);
      draw_tab(style, "Misc", &tab, 3);
      draw_tab(style, "NavEngine", &tab, 4);
      draw_tab(style, "Config", &tab, 5);

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      switch (tab) {
        case 0:
          ImGui::Checkbox("Master", &config.aimbot.master);
          break;
        case 1:
          ImGui::Checkbox("Master", &config.esp.master);
          break;
        case 4:
          ImGui::Checkbox("Master", &config.nav.master);
          break;
        default:
          break;
      }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginChild("Content", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
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
        draw_nav_tab();
        break;
      case 5:
        draw_config_tab();
        break;
      }
    }
    ImGui::EndChild();
  }
  
  ImGui::End();  
}


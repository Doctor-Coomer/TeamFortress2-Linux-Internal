#include "../hacks/esp/esp.hpp"
#include "../hacks/esp/esp_player.cpp"
#include "../hacks/esp/esp_entity.cpp"

#include "../hacks/navbot/draw_navmesh.cpp"

#include <cstring>
#include <unistd.h>
#include <wchar.h>
#include <string>

#include "../gui/config.hpp"

#include "../interfaces/engine.hpp"
#include "../interfaces/surface.hpp"
#include "../interfaces/entity_list.hpp"
#include "../interfaces/netvars.hpp"

#include "../classes/player.hpp"
 
void (*paint_traverse_original)(void*, void*, __int8_t, __int8_t) = NULL;

void* vgui;
const char* get_panel_name(void* panel) {
    void** vtable = *(void ***)vgui;

    const char* (*get_panel_name_fn)(void*, void*) = (const char* (*)(void*, void*))vtable[37];

    return get_panel_name_fn(vgui, panel);
}


void paint_traverse_hook(void* me, void* panel, __int8_t force_repaint, __int8_t allow_force) {
  static bool netvars_initialized_and_dumped = false;
  if (!netvars_initialized_and_dumped) {
    if (client && engine->is_in_game()) {
      netvars::init(client);
      netvars_initialized_and_dumped = true;
    }
  }
  std::string panel_name = get_panel_name(panel);

  // skip the original function to hide elements
  if (config.visuals.removals.scope == true && panel_name == "HudScope") {
    return;
  }

  paint_traverse_original(me, panel, force_repaint, allow_force);

  //print("%s\n", panel_name.c_str());
  
  if (panel_name != "MatSystemTopPanel") {
    return;
  }
  
  if (!engine->is_in_game()) {
    return;
  }

  static int old_font_height = config.debug.font_height;
  static int old_font_weight = config.debug.font_weight;
  
  if (esp_player_font == 0 || old_font_height != config.debug.font_height || old_font_weight != config.debug.font_weight) {
    esp_player_font = surface->text_create_font();
    surface->text_set_font_glyph_set(esp_player_font, "ProggySquare", config.debug.font_height, config.debug.font_weight, 0, 0, 0x0);
  }
  
  if (esp_entity_font == 0 || old_font_height != config.debug.font_height || old_font_weight != config.debug.font_weight) {
    old_font_height = config.debug.font_height;
    old_font_weight = config.debug.font_weight;
    esp_entity_font = surface->text_create_font();
    surface->text_set_font_glyph_set(esp_entity_font, "ProggySquare", config.debug.font_height, config.debug.font_weight, 0, 0, 0x0);
  }
  

  // Aimbot fov
  if (config.aimbot.draw_fov == true && config.aimbot.master == true && config.aimbot.fov < 90) {
    Vec2 screen_size = engine->get_screen_size();

    Player* localplayer = entity_list->get_localplayer();

    //very poor practice.
    float local_fov = localplayer->get_fov();
    if (config.visuals.override_fov == true && (localplayer->is_scoped()) == false) {
      local_fov = config.visuals.custom_fov;
    }
    if (config.visuals.removals.zoom == true) {
      local_fov = localplayer->get_default_fov();
    }
    if (config.visuals.override_fov == true && config.visuals.removals.zoom == true) {
      local_fov = config.visuals.custom_fov;
    }
      
    
    int radius = (tan(config.aimbot.fov / 180 * M_PI) / tan((local_fov / 2) / 180 * M_PI) * (float(screen_size.x) / 2))/1.35;

    surface->set_rgba(255, 255, 255, 255);
    surface->draw_circle(screen_size.x / 2, screen_size.y /2, radius, 55);
  }

  // Navmesh debug visual
  draw_navmesh();

  // Entity and Player esp
  for (unsigned int i = 1; i <= entity_list->get_max_entities(); ++i) {
    if (config.esp.master == false) continue;

    Player* player = entity_list->player_from_index(i);
    if (player == nullptr) continue;

    if (player->get_class_id() == class_id::PLAYER) {
      surface->draw_set_text_font(esp_player_font);
      esp_player(i, player);
    } else {
      surface->draw_set_text_font(esp_entity_font);
      esp_entity(i, player->to_entity());
    }
  }
}

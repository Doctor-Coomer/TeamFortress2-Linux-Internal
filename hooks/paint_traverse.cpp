#include "../hacks/esp/esp.hpp"

#include "../gui/config.hpp"

#include <cstring>
#include <unistd.h>
#include <wchar.h>
#include <string>

#include "../interfaces/engine.hpp"
#include "../interfaces/surface.hpp"
#include "../interfaces/entity_list.hpp"

#include "../interfaces/debug_overlay.hpp"
#include "../hacks/navmesh/navengine.hpp"
#include "../hacks/navmesh/navparser.hpp"
#include "../hacks/navmesh/pathfinder.hpp"

#include "../hacks/navmesh/navparser.cpp"
#include "../hacks/navmesh/navengine.cpp"
#include "../hacks/navmesh/micropather/micropather.cpp"
#include "../hacks/navmesh/pathfinder.cpp"
#include "../hacks/navmesh/navbot/nbcore.cpp"

#include "../classes/player.hpp"

#include "../hacks/esp/esp_player.cpp"
#include "../hacks/esp/esp_entity.cpp"

void (*paint_traverse_original)(void*, void*, __int8_t, __int8_t) = NULL;

void* vgui;
const char* get_panel_name(void* panel) {
    void** vtable = *(void ***)vgui;

    const char* (*get_panel_name_fn)(void*, void*) = (const char* (*)(void*, void*))vtable[37];

    return get_panel_name_fn(vgui, panel);
}




void paint_traverse_hook(void* me, void* panel, __int8_t force_repaint, __int8_t allow_force) {
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
  
  if (esp_player_font == 0) {
    esp_player_font = surface->text_create_font();
    surface->text_set_font_glyph_set(esp_player_font, "ProggySquare", 14, 400, 0, 0, 0x0);
  }

  
  if (esp_entity_font == 0) {
    esp_entity_font = surface->text_create_font();
    surface->text_set_font_glyph_set(esp_entity_font, "ProggySquare", 14, 400, 0, 0, 0x0);
  }

  if (config.aimbot.draw_fov == true && config.aimbot.master == true) {
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

  do {
    if (!engine || !surface || !overlay) break;
    if (!(config.nav.master && config.nav.engine_enabled && config.nav.visualizer_3d)) break;

    (void)nav::EnsureLoadedForCurrentLevel();

    if (!nav::IsLoaded()) break;

    const nav::Mesh* mesh = nav::GetMesh();
    if (!mesh) break;

    surface->set_rgba(0, 255, 180, 160);
    for (const auto& a : mesh->areas) {
      Vec3 nw{a.nw[0], a.nw[1], a.nw[2]};
      Vec3 ne{a.se[0], a.nw[1], a.ne_z};
      Vec3 se{a.se[0], a.se[1], a.se[2]};
      Vec3 sw{a.nw[0], a.se[1], a.sw_z};

      Vec3 snw, sne, sse, ssw;
      if (!(overlay->world_to_screen(&nw, &snw) && overlay->world_to_screen(&ne, &sne) &&
            overlay->world_to_screen(&se, &sse) && overlay->world_to_screen(&sw, &ssw))) {
        continue;
      }

      surface->draw_line((int)snw.x, (int)snw.y, (int)sne.x, (int)sne.y);
      surface->draw_line((int)sne.x, (int)sne.y, (int)sse.x, (int)sse.y);
      surface->draw_line((int)sse.x, (int)sse.y, (int)ssw.x, (int)ssw.y);
      surface->draw_line((int)ssw.x, (int)ssw.y, (int)snw.x, (int)snw.y);
    }

    do {
      const std::vector<uint32_t>* ids = nullptr;
      size_t next_idx = 0;
      uint32_t goal_id = 0;
      nav::Visualizer_GetPath(&ids, &next_idx, &goal_id);
      if (!ids || ids->empty()) break;

      std::vector<Vec3> pts;
      pts.reserve(ids->size());
      for (uint32_t id : *ids) {
        const nav::Area* a = nav::path::GetAreaById(id);
        if (!a) continue;
        float c[3];
        nav::path::GetAreaCenter(a, c);
        pts.push_back(Vec3{c[0], c[1], c[2]});
      }
      if (pts.size() < 2) break;

      surface->set_rgba(255, 230, 0, 215);
      for (size_t i = 0; i + 1 < pts.size(); ++i) {
        Vec3 s0, s1;
        if (overlay->world_to_screen(&pts[i], &s0) && overlay->world_to_screen(&pts[i+1], &s1)) {
          surface->draw_line((int)s0.x, (int)s0.y, (int)s1.x, (int)s1.y);
        }
      }

      if (next_idx < pts.size()) {
        Vec3 snext; if (overlay->world_to_screen(&pts[next_idx], &snext)) {
          surface->set_rgba(50, 200, 255, 230);
          surface->draw_circle((int)snext.x, (int)snext.y, 5, 24);
        }
      }
      if (goal_id) {
        const nav::Area* ga = nav::path::GetAreaById(goal_id);
        if (ga) {
          float gc[3]; nav::path::GetAreaCenter(ga, gc);
          Vec3 g{gc[0], gc[1], gc[2]}, sg;
          if (overlay->world_to_screen(&g, &sg)) {
            surface->set_rgba(255, 90, 40, 230);
            surface->draw_circle((int)sg.x, (int)sg.y, 6, 26);
          }
        }
      }
    } while (false);
  } while (false);
}

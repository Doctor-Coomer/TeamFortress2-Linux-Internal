#include "navmesh.hpp"

#include "../esp/esp.hpp"

#include "../../gui/config.hpp"

#include "../../interfaces/render_view.hpp"
#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/surface.hpp"

#include "../../classes/player.hpp"

#include "../../vec.hpp"

#include "../../print.hpp"

void draw_navmesh() {  
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) return;

  Vec3 location = localplayer->get_origin();
  
  if (mesh.map_name != "" && !mesh.areas.empty() && path.path_ids.size() > path.next_index) {  
    surface->set_rgba(0, 255, 180, 160);
    surface->draw_set_text_font(esp_entity_font);
    surface->draw_set_text_color(0, 255, 180, 255);

    if (config.debug.debug_draw_navmesh == true) {
      for (Area &area : mesh.areas) {      
	Vec3 nw{area.nw[0], area.nw[1], area.nw[2]};
	Vec3 ne{area.se[0], area.nw[1], area.ne_z};
	Vec3 se{area.se[0], area.se[1], area.se[2]};
	Vec3 sw{area.nw[0], area.se[1], area.sw_z};

	Vec3 snw, sne, sse, ssw;
	if (!(render_view->world_to_screen(&nw, &snw) && render_view->world_to_screen(&ne, &sne) &&
	      render_view->world_to_screen(&se, &sse) && render_view->world_to_screen(&sw, &ssw)))
	  {
	    continue;
	  }
      
	surface->draw_line(snw.x, snw.y, sne.x, sne.y);
	surface->draw_line(sne.x, sne.y, sse.x, sse.y);
	surface->draw_line(sse.x, sse.y, ssw.x, ssw.y);
	surface->draw_line(ssw.x, ssw.y, snw.x, snw.y);

	Vec3 area_center = area.center();
	Vec3 area_center_screen;
	render_view->world_to_screen(&area_center, &area_center_screen);
	surface->draw_set_text_pos(area_center_screen.x - surface->get_string_width(esp_entity_font, std::to_wstring(area.id).c_str())*0.5, area_center_screen.y);
	surface->draw_print_text(std::to_wstring(area.id).c_str(), std::to_wstring(area.id).length());
      }
    }
    
    if (config.debug.debug_draw_navbot_path == true && !path.path_ids.empty()) {
      for (unsigned int i = 0; i < path.path_ids.size()-1; ++i) {
	Area* a = mesh.id_to_area(path.path_ids[i]);
	Area* b = mesh.id_to_area(path.path_ids[i+1]);
	if (a != nullptr || b != nullptr) {
	  Vec3 a_center = a->center();
	  Vec3 a_center_screen;
	  Vec3 b_center = b->center();
	  Vec3 b_center_screen;
	  if (render_view->world_to_screen(&b_center, &b_center_screen) && render_view->world_to_screen(&a_center, &a_center_screen)) {
	    surface->set_rgba(35, 255, 0, 255);
	    surface->draw_line(a_center_screen.x, a_center_screen.y, b_center_screen.x, b_center_screen.y);
	  }
	}
      }
    }
    
    Area* current_area = mesh.best_area_from_xyz(location);
    if (config.debug.debug_draw_navbot_current_area == true && current_area != nullptr) {
      Area* a = mesh.id_to_area(current_area->id);
      if (a != nullptr) {
	Vec3 a_center = a->center();
	Vec3 a_center_screen;
	if (render_view->world_to_screen(&a_center, &a_center_screen)) {
	  surface->set_rgba(255, 50, 160, 255);	
	  surface->draw_circle(a_center_screen.x, a_center_screen.y, 20, 50);
	}
      }
    }


    if (config.debug.debug_draw_navbot_goal == true) {
      Area* a = mesh.id_to_area(path.goal_id);
      if (a != nullptr) {
	Vec3 a_center = a->center();
	Vec3 a_center_screen;
	if (render_view->world_to_screen(&a_center, &a_center_screen)) {
	  surface->set_rgba(255, 200, 30, 255);
	  surface->draw_circle(a_center_screen.x, a_center_screen.y, 20, 50);
	}
      }
    }

    if (config.debug.debug_draw_navbot_next_area == true) {
      Area* next_area = mesh.id_to_area(path.path_ids[path.next_index]);
      if (next_area != nullptr) {
	Vec3 a_center = next_area->center();
	Vec3 a_center_screen;
	if (render_view->world_to_screen(&a_center, &a_center_screen)) {
	  surface->set_rgba(70, 255, 200, 255);
	  surface->draw_circle(a_center_screen.x, a_center_screen.y, 20, 50);
	}	
      }
    }
  }

}

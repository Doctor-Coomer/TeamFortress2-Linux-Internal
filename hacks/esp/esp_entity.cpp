#include "esp.hpp"

#include <stdlib.h>
#include <string>

#include "../../gui/config.hpp"

#include "../../interfaces/surface.hpp"
#include "../../interfaces/render_view.hpp"

#include "../../classes/player.hpp"
#include "../../classes/building.hpp"

#include "../../vec.hpp"
#include "../../print.hpp"

void box_esp_entity(Vec3 screen, Entity* entity, Player* localplayer) {
  if (config.esp.pickup.box == true && entity->get_pickup_type() != pickup_type::UNKNOWN) {
    if (entity->is_dormant()) return;
    surface->set_rgba(255, 255, 255, 255);
    surface->draw_outlined_rect(screen.x-5, screen.y-5, screen.x+5, screen.y+5);
  }

  if (config.esp.intelligence.box == true && entity->get_class_id() == class_id::CAPTURE_FLAG && entity->get_owner_entity() != localplayer) {
    Vec3 location = entity->get_origin();

    Vec3 screen_offset_top;
    Vec3 location_offset_top = {location.x, location.y, location.z + 6};
    render_view->world_to_screen(&location_offset_top, &screen_offset_top);

    Vec3 screen_offset_bottom;
    Vec3 location_offset_bottom = {location.x, location.y, location.z - 6};
    render_view->world_to_screen(&location_offset_bottom, &screen_offset_bottom);    
    
    RGBA color = {255, 255, 255, 255};
    if (entity->get_team() == tf_team::TEAM_BLU) color = RGBA{0, 0, 255, 255};
    if (entity->get_team() == tf_team::TEAM_RED) color = RGBA{255, 0, 0, 255};
    draw_outline_rectangle(screen_offset_bottom, screen_offset_top, 1.75, color);
  }
  
  if (config.esp.buildings.box == true && entity->is_building() && (entity->get_team() != localplayer->get_team() || (entity->get_team() == localplayer->get_team() && config.esp.buildings.team == true))) {
    Building* building = (Building*)entity;
    if (building->is_carried()) return;
    if (entity->is_dormant()) return;
    
    Vec3 screen_offset;
    float box_offset_fraction = 0;
    
    switch (building->get_class_id()) {
    case class_id::DISPENSER:
      {
	Vec3 location = building->get_origin();
	Vec3 location_offset = {location.x, location.y, location.z + 58};
	render_view->world_to_screen(&location_offset, &screen_offset);

	box_offset_fraction = 0.25;
	break;
      }
    case class_id::SENTRY:
      {
	Vec3 location = building->get_origin();

	float z_offset = 0;
	switch (building->get_building_level()) {
	case 1:
	  z_offset = 45;
	  box_offset_fraction = 0.30;
	  break;
	case 2:
	  z_offset = 55;
	  box_offset_fraction = 0.60;
	  break;
	case 3:
	  z_offset = 75;
	  box_offset_fraction = 0.50;
	  break;
	}

	Vec3 location_offset = {location.x, location.y, location.z + z_offset};
	render_view->world_to_screen(&location_offset, &screen_offset);
	break;
      }
    case class_id::TELEPORTER:
      {
	Vec3 location = building->get_origin();
	Vec3 location_offset = {location.x, location.y, location.z + 15};
	render_view->world_to_screen(&location_offset, &screen_offset);

	box_offset_fraction = 1.8;
	break;
      }
    }

    RGBA color = config.esp.player.enemy_color.to_RGBA(); // Use player color for now
    if (building->get_team() == localplayer->get_team()) color = config.esp.player.team_color.to_RGBA();
    
    draw_outline_rectangle(screen, screen_offset, box_offset_fraction, color);
  }
}

void health_bar_esp_entity(Vec3 screen, Entity* entity, Player* localplayer) {
  if (config.esp.buildings.health_bar == true && entity->is_building() && (entity->get_team() != localplayer->get_team() || (entity->get_team() == localplayer->get_team() && config.esp.buildings.team == true))) {
    Building* building = (Building*)entity;
    if (building->is_carried()) return;
    if (entity->is_dormant()) return;

    float box_offset = 0;
    switch (building->get_class_id()) {
    case class_id::SENTRY:
      {
	Vec3 location = building->get_origin();

	float z_offset = 0;
	float box_offset_fraction = 0;
	switch (building->get_building_level()) {
	case 1:
	  z_offset = 45;
	  box_offset_fraction = 0.30;
	  break;
	case 2:
	  z_offset = 55;
	  box_offset_fraction = 0.60;
	  break;
	case 3:
	  z_offset = 75;
	  box_offset_fraction = 0.50;
	  break;
	}
    
	Vec3 location_offset = {location.x, location.y, location.z + z_offset};
	Vec3 screen_offset;
	render_view->world_to_screen(&location_offset, &screen_offset);    

	box_offset = (screen.y - screen_offset.y)*box_offset_fraction;
	break;
      }
    case class_id::DISPENSER:
      {    
	Vec3 location = building->get_origin();
	Vec3 location_offset = {location.x, location.y, location.z + 58};
	Vec3 screen_offset;
	render_view->world_to_screen(&location_offset, &screen_offset);

	box_offset = (screen.y - screen_offset.y)*0.25;    
	break;
      }
    case class_id::TELEPORTER:
      {
    
	Vec3 location = building->get_origin();
	Vec3 location_offset = {location.x, location.y, location.z + 15};
	Vec3 screen_offset;
	render_view->world_to_screen(&location_offset, &screen_offset);

	box_offset = (screen.y - screen_offset.y)*1.8;
	break;
      }
    }

    surface->set_rgba(0, 0, 0, 255);
    surface->draw_line(screen.x - box_offset - 1, screen.y + 3, screen.x + box_offset + 2, screen.y + 3);
    surface->draw_line(screen.x - box_offset - 1, screen.y + 4, screen.x + box_offset + 2, screen.y + 4);
    surface->draw_line(screen.x - box_offset - 1, screen.y + 5, screen.x + box_offset + 2, screen.y + 5);
    
    int xdelta = (box_offset*2) * (1.f - (float(building->get_health()) / building->get_max_health()));    

    if (building->get_health() > building->get_max_health()) { // over healed (the building some how???)
      surface->set_rgba(0, 255, 255, 255);
      xdelta = 0;
    }
    else if (building->get_health() <= building->get_max_health() && building->get_health() >= (building->get_max_health()*.9))
      surface->set_rgba(0, 255, 0, 255);
    else if (building->get_health() < (building->get_max_health()*.9) && building->get_health() > (building->get_max_health()*.6))
      surface->set_rgba(90, 255, 0, 255);
    else if (building->get_health() <= (building->get_max_health()*.6) && building->get_health() > (building->get_max_health()*.35))
      surface->set_rgba(255, 100, 0, 255);
    else if (building->get_health() <= (building->get_max_health()*.35))
      surface->set_rgba(255, 0, 0, 255);
    
    surface->draw_line(screen.x - box_offset, screen.y + 4, screen.x + box_offset - xdelta + 1, screen.y + 4);
  }
}


void name_esp_entity(Vec3 screen, Entity* entity, Player* localplayer) {

  if (config.esp.pickup.name == true && entity->get_pickup_type() != pickup_type::UNKNOWN) {
    if (entity->is_dormant()) return;
    surface->draw_set_text_color(255, 255, 255, 255);
    
    if (entity->get_pickup_type() == pickup_type::AMMOPACK) {
      surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"AMMO")*0.5), screen.y);
      surface->draw_print_text(L"AMMO", wcslen(L"AMMO"));
    } else if (entity->get_pickup_type() == pickup_type::MEDKIT) {
      surface->draw_set_text_color(0, 255, 25, 255);
      surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"HEALTH")*0.5), screen.y);
      surface->draw_print_text(L"HEALTH", wcslen(L"HEALTH"));
    }
  }

  if (config.esp.intelligence.name == true && entity->get_class_id() == class_id::CAPTURE_FLAG) {
    Vec3 location = entity->get_origin();

    Vec3 screen_offset_bottom;
    Vec3 location_offset_bottom = {location.x, location.y, location.z - 6};
    render_view->world_to_screen(&location_offset_bottom, &screen_offset_bottom);    
    
    RGBA color = {255, 255, 255, 255};
    if (entity->get_team() == tf_team::TEAM_BLU) color = RGBA{0, 0, 255, 255};
    if (entity->get_team() == tf_team::TEAM_RED) color = RGBA{255, 0, 0, 255};
    surface->draw_set_text_color(color);

    surface->draw_set_text_pos(screen_offset_bottom.x - surface->get_string_width(esp_entity_font, L"FLAG")*0.5, screen_offset_bottom.y + 1);
    surface->draw_print_text(L"FLAG", wcslen(L"FLAG"));
  }
  
  if (config.esp.buildings.name == true && entity->is_building() && (entity->get_team() != localplayer->get_team() || (entity->get_team() == localplayer->get_team() && config.esp.buildings.team == true))) {
    Building* building = (Building*)entity;
    if (building->is_carried()) return;
    if (entity->is_dormant()) return;
    
    surface->draw_set_text_color(255, 255, 255, 255);
    switch (building->get_class_id()) {
    case class_id::DISPENSER:
      {
	surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"DISPENSER")*0.5), screen.y + (config.esp.buildings.health_bar ? 5 : 0));
	surface->draw_print_text(L"DISPENSER", wcslen(L"DISPENSER"));
	break;
      }
    case class_id::SENTRY:
      {
	surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"SENTRY")*0.5), screen.y + (config.esp.buildings.health_bar ? 5 : 0));
	surface->draw_print_text(L"SENTRY", wcslen(L"SENTRY"));
	break;
      }
    case class_id::TELEPORTER:
      {
	surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"TELEPORTER")*0.5), screen.y + (config.esp.buildings.health_bar ? 5 : 0));
	surface->draw_print_text(L"TELEPORTER", wcslen(L"TELEPORTER"));
	break;
      }
    }

  }
  
  if (config.debug.debug_render_all_entities == true) {
    std::string model_name = entity->get_model_name();
      
    wchar_t model_name_w[64];
    size_t len = std::mbstowcs(model_name_w, model_name.c_str(), 64);
    if (len == (size_t)-1) return;

    std::wstring a
      = L"Model Path: " + std::wstring(model_name_w)
      + L"\nClass ID: " + std::to_wstring(entity->get_class_id())
      + L"\nEntity Index: " + std::to_wstring(entity->get_index());

    surface->draw_set_text_color(255, 255, 255, 255);
    surface->draw_set_text_pos(screen.x, screen.y);
    surface->draw_print_text(a.c_str(), wcslen(a.c_str()));

  }

}

void timer_esp_entity() {

  for (unsigned int i = 0; i < pickup_item_cache.size(); ++i) {
    PickupItem pickup_item = pickup_item_cache[i];
    float time_delta = pickup_item.time - global_vars->curtime;
    if (time_delta < 0) {
      pickup_item_cache.erase(pickup_item_cache.begin()+i);
      continue;
    }
    
    Vec3 screen;
    if (!render_view->world_to_screen(&pickup_item.location, &screen)) continue;

    std::wstring time_delta_str = std::to_wstring(time_delta);

    time_delta_str.resize(4);
    
    surface->draw_set_text_color(255, 255, 255, 255);
    surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, time_delta_str.c_str())*0.5), screen.y);
    surface->draw_print_text(time_delta_str.c_str(), wcslen(time_delta_str.c_str()));
  }

}

void esp_entity(unsigned int i, Entity* entity) {
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) return;

  Vec3 location = entity->get_origin();
  Vec3 screen;
  if (!render_view->world_to_screen(&location, &screen)) return;

  box_esp_entity(screen, entity, localplayer);
  health_bar_esp_entity(screen, entity, localplayer);
  name_esp_entity(screen, entity, localplayer);
  timer_esp_entity();
}

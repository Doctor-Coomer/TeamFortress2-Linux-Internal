#include "esp.hpp"

#include <cstdlib>
#include <string>

#include "../../classes/player.hpp"
#include "../../classes/building.hpp"

#include "../../gui/config.hpp"

#include "../../vec.hpp"

#include "../../interfaces/surface.hpp"
#include "../../interfaces/render_view.hpp"

#include "../../print.hpp"

void box_esp_entity(Vec3 screen, Entity *entity, Player *localplayer) {
    if (config.esp.pickup.box && entity->get_pickup_type() != UNKNOWN) {
        surface->set_rgba(255, 255, 255, 255);
        surface->draw_outlined_rect(screen.x - 5, screen.y - 5, screen.x + 5, screen.y + 5);
    }

    if (config.esp.buildings.box && entity->is_building() && (
            entity->get_team() != localplayer->get_team() || (
                entity->get_team() == localplayer->get_team() && config.esp.buildings.team))) {
        Building *building = (Building *) entity;
        if (building->is_carried()) return;

        Vec3 screen_offset;
        float box_offset_fraction = 0;

        switch (building->get_class_id()) {
            case DISPENSER: {
                Vec3 location = building->get_origin();
                Vec3 location_offset = {location.x, location.y, location.z + 58};
                render_view->world_to_screen(&location_offset, &screen_offset);

                box_offset_fraction = 0.25;
                break;
            }
            case SENTRY: {
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
                    default: break;
                }

                Vec3 location_offset = {location.x, location.y, location.z + z_offset};
                render_view->world_to_screen(&location_offset, &screen_offset);\
                break;
            }
            case TELEPORTER: {
                Vec3 location = building->get_origin();
                Vec3 location_offset = {location.x, location.y, location.z + 15};
                render_view->world_to_screen(&location_offset, &screen_offset);

                box_offset_fraction = 1.8;
                break;
            }
            default: break;
        }

        RGBA color = config.esp.player.enemy_color.to_RGBA(); // Use player color for now
        if (building->get_team() == localplayer->get_team()) color = config.esp.player.team_color.to_RGBA();

        draw_outline_rectangle(screen, screen_offset, box_offset_fraction, color);
    }
}

void health_bar_esp_entity(Vec3 screen, Entity *entity, Player *localplayer) {
    if (config.esp.buildings.health_bar && entity->is_building() && (
            entity->get_team() != localplayer->get_team() || (
                entity->get_team() == localplayer->get_team() && config.esp.buildings.team))) {
        Building *building = (Building *) entity;
        if (building->is_carried()) return;

        float box_offset = 0;
        switch (building->get_class_id()) {
            case SENTRY: {
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
                    default: break;
                }

                Vec3 location_offset = {location.x, location.y, location.z + z_offset};
                Vec3 screen_offset;
                render_view->world_to_screen(&location_offset, &screen_offset);

                box_offset = (screen.y - screen_offset.y) * box_offset_fraction;
                break;
            }
            case DISPENSER: {
                Vec3 location = building->get_origin();
                Vec3 location_offset = {location.x, location.y, location.z + 58};
                Vec3 screen_offset;
                render_view->world_to_screen(&location_offset, &screen_offset);

                box_offset = (screen.y - screen_offset.y) * 0.25;
                break;
            }
            case TELEPORTER: {
                Vec3 location = building->get_origin();
                Vec3 location_offset = {location.x, location.y, location.z + 15};
                Vec3 screen_offset;
                render_view->world_to_screen(&location_offset, &screen_offset);

                box_offset = (screen.y - screen_offset.y) * 1.8;
                break;
            }
            default: break;
        }

        surface->set_rgba(0, 0, 0, 255);
        surface->draw_line(screen.x - box_offset - 1, screen.y + 3, screen.x + box_offset + 2, screen.y + 3);
        surface->draw_line(screen.x - box_offset - 1, screen.y + 4, screen.x + box_offset + 2, screen.y + 4);
        surface->draw_line(screen.x - box_offset - 1, screen.y + 5, screen.x + box_offset + 2, screen.y + 5);

        int xdelta = (box_offset * 2) * (1.f - (float(building->get_health()) / building->get_max_health()));

        if (building->get_health() > building->get_max_health()) {
            // over healed (the building somehow???)
            surface->set_rgba(0, 255, 255, 255);
            xdelta = 0;
        } else if (building->get_health() <= building->get_max_health() && building->get_health() >= (
                       building->get_max_health() * .9))
            surface->set_rgba(0, 255, 0, 255);
        else if (building->get_health() < (building->get_max_health() * .9) && building->get_health() > (
                     building->get_max_health() * .6))
            surface->set_rgba(90, 255, 0, 255);
        else if (building->get_health() <= (building->get_max_health() * .6) && building->get_health() > (
                     building->get_max_health() * .35))
            surface->set_rgba(255, 100, 0, 255);
        else if (building->get_health() <= (building->get_max_health() * .35))
            surface->set_rgba(255, 0, 0, 255);

        surface->draw_line(screen.x - box_offset, screen.y + 4, screen.x + box_offset - xdelta + 1, screen.y + 4);
    }
}


void name_esp_entity(Vec3 screen, Entity *entity, Player *localplayer) {
    if (config.esp.pickup.name && entity->get_pickup_type() != UNKNOWN) {
        surface->draw_set_text_color(255, 255, 255, 255);

        if (entity->get_pickup_type() == AMMOPACK) {
            surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"AMMO") * 0.5),
                                       screen.y);
            surface->draw_print_text(L"AMMO", wcslen(L"AMMO"));
        } else if (entity->get_pickup_type() == MEDKIT) {
            surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"HEALTH") * 0.5),
                                       screen.y);
            surface->draw_print_text(L"HEALTH", wcslen(L"HEALTH"));
        }
    }


    if (config.esp.buildings.name && entity->is_building() && (
            entity->get_team() != localplayer->get_team() || (
                entity->get_team() == localplayer->get_team() && config.esp.buildings.team))) {
        Building *building = (Building *) entity;
        if (building->is_carried()) return;

        surface->draw_set_text_color(255, 255, 255, 255);
        switch (building->get_class_id()) {
            case DISPENSER: {
                surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"DISPENSER") * 0.5),
                                           screen.y + (config.esp.buildings.health_bar ? 5 : 0));
                surface->draw_print_text(L"DISPENSER", wcslen(L"DISPENSER"));
                break;
            }
            case SENTRY: {
                surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"SENTRY") * 0.5),
                                           screen.y + (config.esp.buildings.health_bar ? 5 : 0));
                surface->draw_print_text(L"SENTRY", wcslen(L"SENTRY"));
                break;
            }
            case TELEPORTER: {
                surface->draw_set_text_pos(screen.x - (surface->get_string_width(esp_entity_font, L"TELEPORTER") * 0.5),
                                           screen.y + (config.esp.buildings.health_bar ? 5 : 0));
                surface->draw_print_text(L"TELEPORTER", wcslen(L"TELEPORTER"));
                break;
            }
            default: break;
        }
    }

    if (config.debug.debug_render_all_entities) {
        std::string model_name = entity->get_model_name();

        wchar_t model_name_w[64];
        size_t len = std::mbstowcs(model_name_w, model_name.c_str(), 64);
        if (len == (size_t) -1) return;

        std::wstring a = std::wstring(model_name_w);

        a += L" " + std::to_wstring(entity->get_class_id());

        surface->draw_set_text_pos(screen.x, screen.y);
        surface->draw_print_text(a.c_str(), wcslen(a.c_str()));
    }
}

void esp_entity(unsigned int i, Entity *entity) {
    Player *localplayer = entity_list->get_localplayer();
    if (localplayer == nullptr) return;

    Vec3 location = entity->get_origin();
    Vec3 screen;
    if (!render_view->world_to_screen(&location, &screen)) return;

    box_esp_entity(screen, entity, localplayer);
    health_bar_esp_entity(screen, entity, localplayer);
    name_esp_entity(screen, entity, localplayer);
}

#include "../gui/config.hpp"

#include "../interfaces/entity_list.hpp"
#include "../interfaces/surface.hpp"
#include "../interfaces/engine.hpp"

#include "../classes/player.hpp"

#include "../hacks/esp/esp_player.cpp"
#include "../hacks/esp/esp_entity.cpp"

void (*enginevgui_paint_original)(void *, int);

enum paint_mode {
    paint_uipanels = 1 << 0,
    paint_ingamepanels = 1 << 1,
    paint_cursor = 1 << 2
};

void enginevgui_paint_hook(void *me, paint_mode mode) {
    if (mode & paint_ingamepanels) {
        surface->start_drawing();
        /*{
            // font init
            static int old_font_height = config.debug.font_height;
            static int old_font_weight = config.debug.font_weight;

            if (esp_player_font == 0 || old_font_height != config.debug.font_height || old_font_weight != config.debug.
                font_weight) {
                esp_player_font = surface->text_create_font();
                surface->text_set_font_glyph_set(esp_player_font, "Noto Sans", config.debug.font_height,
                                                 config.debug.font_weight, 0, 0, 0x010);
            }

            if (esp_entity_font == 0 || old_font_height != config.debug.font_height || old_font_weight != config.debug.
                font_weight) {
                old_font_height = config.debug.font_height;
                old_font_weight = config.debug.font_weight;
                esp_entity_font = surface->text_create_font();
                surface->text_set_font_glyph_set(esp_entity_font, "Noto Sans", config.debug.font_height,
                                                 config.debug.font_weight, 0, 0, 0x010);
            }
        }*/
        if (Player *localplayer = entity_list->get_localplayer(); engine->is_in_game()) {
            static int old_font_height = config.debug.font_height;
            static int old_font_weight = config.debug.font_weight;

            if (esp_player_font == 0 || old_font_height != config.debug.font_height || old_font_weight != config.debug.
                font_weight) {
                esp_player_font = surface->text_create_font();
                surface->text_set_font_glyph_set(esp_player_font, "ProggySquare", config.debug.font_height,
                                                 config.debug.font_weight, 0, 0, 0x0);
            }

            if (esp_entity_font == 0 || old_font_height != config.debug.font_height || old_font_weight != config.debug.
                font_weight) {
                old_font_height = config.debug.font_height;
                old_font_weight = config.debug.font_weight;
                esp_entity_font = surface->text_create_font();
                surface->text_set_font_glyph_set(esp_entity_font, "ProggySquare", config.debug.font_height,
                                                 config.debug.font_weight, 0, 0, 0x0);
            }


            if (config.aimbot.draw_fov == true && config.aimbot.master == true) {
                Vec2 screen_size = engine->get_screen_size();

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


                int radius = (tan(config.aimbot.fov / 180 * M_PI) / tan((local_fov / 2) / 180 * M_PI) * (
                                  float(screen_size.x) / 2)) / 1.35;

                surface->set_rgba(255, 255, 255, 255);
                surface->draw_circle(screen_size.x / 2, screen_size.y / 2, radius, 55);
            }


            for (unsigned int i = 1; i <= entity_list->get_max_entities(); ++i) {
                if (!config.esp.master) continue;

                Player *player = entity_list->player_from_index(i);
                if (!player) continue;

                if (player->get_class_id() == PLAYER) {
                    surface->draw_set_text_font(esp_player_font);
                    esp_player(i, player);
                } else {
                    surface->draw_set_text_font(esp_entity_font);
                    esp_entity(i, player->to_entity());
                }
            }
        }
        surface->finish_drawing();
    }

    enginevgui_paint_original(me, mode);
}

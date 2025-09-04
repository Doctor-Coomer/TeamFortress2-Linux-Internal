#ifndef ESP_HPP
#define ESP_HPP

#include "../../interfaces/surface.hpp"

inline static unsigned long esp_player_font;
inline static unsigned long esp_entity_font;

static void draw_outline_rectangle(Vec3 screen, Vec3 screen_offset, float width_fraction, RGBA color) {
    float box_offset = (screen.y - screen_offset.y) * width_fraction;

    /* shadow box */
    surface->set_rgba(0, 0, 0, 255);

    //right side
    surface->draw_line(screen.x + box_offset + 1, screen.y + 1, screen.x + box_offset + 1, screen_offset.y - 1);
    surface->draw_line(screen.x + box_offset - 1, screen.y + 1, screen.x + box_offset - 1, screen_offset.y - 1);

    //left side
    surface->draw_line(screen.x - box_offset + 1, screen.y + 1, screen.x - box_offset + 1, screen_offset.y - 1);
    surface->draw_line(screen.x - box_offset - 1, screen.y + 1, screen.x - box_offset - 1, screen_offset.y - 1);

    //top
    surface->draw_line(screen.x - box_offset - 1, screen_offset.y + 1, screen.x + box_offset + 2, screen_offset.y + 1);
    surface->draw_line(screen.x - box_offset - 1, screen_offset.y - 1, screen.x + box_offset + 2, screen_offset.y - 1);

    //bottom
    surface->draw_line(screen.x - box_offset - 1, screen.y + 1, screen.x + box_offset + 1, screen.y + 1);
    surface->draw_line(screen.x - box_offset - 1, screen.y - 1, screen.x + box_offset + 1, screen.y - 1);
    /* shadow box */


    /* actual box */
    surface->set_rgba(color.r, color.g, color.b, color.a);

    //right side
    surface->draw_line(screen.x + box_offset, screen.y, screen.x + box_offset, screen_offset.y);

    //left side
    surface->draw_line(screen.x - box_offset, screen.y, screen.x - box_offset, screen_offset.y);

    //top
    surface->draw_line(screen.x - box_offset, screen_offset.y, screen.x + box_offset + 1, screen_offset.y);

    //bottom
    surface->draw_line(screen.x - box_offset, screen.y, screen.x + box_offset, screen.y);
    /* actual box */
}

#endif

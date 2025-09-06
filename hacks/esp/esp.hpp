#ifndef ESP_HPP
#define ESP_HPP

#include "../../interfaces/surface.hpp"

inline static unsigned long esp_player_font;
inline static unsigned long esp_entity_font;

static void draw_outline_rectangle(const Vec3 screen, const Vec3 screen_offset, const float width_fraction,
                                   const RGBA color) {
    const float box_offset = (screen.y - screen_offset.y) * width_fraction;

    surface->set_rgba(0, 0, 0, 255);
    surface->draw_line(screen.x + box_offset + 1, screen.y + 1, screen.x + box_offset + 1, screen_offset.y - 1);
    surface->draw_line(screen.x + box_offset - 1, screen.y + 1, screen.x + box_offset - 1, screen_offset.y - 1);
    surface->draw_line(screen.x - box_offset + 1, screen.y + 1, screen.x - box_offset + 1, screen_offset.y - 1);
    surface->draw_line(screen.x - box_offset - 1, screen.y + 1, screen.x - box_offset - 1, screen_offset.y - 1);
    surface->draw_line(screen.x - box_offset - 1, screen_offset.y + 1, screen.x + box_offset + 2, screen_offset.y + 1);
    surface->draw_line(screen.x - box_offset - 1, screen_offset.y - 1, screen.x + box_offset + 2, screen_offset.y - 1);
    surface->draw_line(screen.x - box_offset - 1, screen.y + 1, screen.x + box_offset + 1, screen.y + 1);
    surface->draw_line(screen.x - box_offset - 1, screen.y - 1, screen.x + box_offset + 1, screen.y - 1);

    surface->set_rgba(color.r, color.g, color.b, color.a);
    surface->draw_line(screen.x + box_offset, screen.y, screen.x + box_offset, screen_offset.y);
    surface->draw_line(screen.x - box_offset, screen.y, screen.x - box_offset, screen_offset.y);
    surface->draw_line(screen.x - box_offset, screen_offset.y, screen.x + box_offset, screen_offset.y);
    surface->draw_line(screen.x - box_offset, screen.y, screen.x + box_offset, screen.y);
}

#endif

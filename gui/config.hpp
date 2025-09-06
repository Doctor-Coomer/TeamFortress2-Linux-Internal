#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_keyboard.h>

#include "../vec.hpp"

struct button {
    int ibutton;
    bool waiting = false;
};

struct Aim {
    bool master = true;

    bool auto_shoot = true;

    bool silent = true;

    button key = {.ibutton = -SDL_BUTTON_X2};
    bool use_key = true;

    float fov = 20;
    bool draw_fov = true;

    bool auto_scope = false;

    bool ignore_friends = true;
};

struct Esp {
    bool master = true;

    struct Player {
        RGBA_float enemy_color = {.r = 1, .g = 0.501960784, .b = 0, .a = 1};
        RGBA_float team_color = {.r = 1, .g = 1, .b = 1, .a = 1};
        RGBA_float friend_color = {.r = 0, .g = 0.862745098, .b = 0.31372549, .a = 1};

        bool box = true;
        bool health_bar = true;
        bool name = true;

        struct Flags {
            bool target_indicator = true;
            bool friend_indicator = true;
        } flags;

        bool friends = true;
        bool team = false;
    } player;

    struct Pickup {
        bool box = false;
        bool name = true;
    } pickup;

    struct Buildings {
        bool box = true;
        bool health_bar = true;
        bool name = true;

        bool team = false;
    } buildings;
};

struct Visuals {
    struct Removals {
        bool scope = true;
        bool zoom = false;
    } removals;

    struct Thirdperson {
        button key = {.ibutton = -SDL_BUTTON_X1};
        bool enabled = true;
        float z = 250.0f;
        float y = 20.0f;
        float x = 0;
    } thirdperson;

    bool override_fov = true;
    float custom_fov = 110;
};

struct Misc {
    bool bhop = true;
    bool bypasspure = true;
    bool no_push = true;
};

struct Debug {
    int font_height = 12;
    int font_weight = 0;
    bool debug_render_all_entities = false;
};

struct Config {
    Aim aimbot;
    Esp esp;
    Visuals visuals;
    Misc misc;
    Debug debug;
};

inline static Config config;


static bool is_button_down(const button button) {
    if (button.ibutton >= 0) {
        if (const uint8_t *keys = SDL_GetKeyboardState(nullptr); keys[button.ibutton] == 1)
            return true;

        return false;
    } else {
        if (const Uint32 mouse_state = SDL_GetMouseState(nullptr, nullptr); mouse_state & SDL_BUTTON(-button.ibutton))
            return true;

        return false;
    }

    return false;
}

#endif

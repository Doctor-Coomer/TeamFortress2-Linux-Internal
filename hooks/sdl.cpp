#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_syswm.h>
#include <GL/glew.h>

#include "../imgui/dearimgui.hpp"

#include "../interfaces/surface.hpp"

#include "../print.hpp"

#include "../gui/menu.hpp"
#include "../gui/indicators.hpp"

bool (*poll_event_original)(SDL_Event*) = NULL;
int  (*peep_events_original)(SDL_Event*, int, SDL_eventaction, int, int) = NULL;
void (*swap_window_original)(void*) = NULL;
Uint32 (*get_window_flags_original)(SDL_Window*) = NULL;
SDL_bool (*get_window_WM_info_original)(SDL_Window* window, SDL_SysWMinfo* info) = NULL;
void (*get_window_size_original)(SDL_Window* window, int* w, int* h) = NULL;

// This filters key events to the game
int SDLCALL event_filter(void* userdata, SDL_Event* event) {
  if (menu_focused == false) return 1; // Don't filter anything if the menu is closed

  
  if (sdl_window != nullptr && ImGui::IsImGuiFullyInitialized())
    ImGui_ImplSDL2_ProcessEvent(event);

  get_input(event);

  // Allow keys to be released
  if (event->type == SDL_KEYUP) {
    return 1;
  }
  
  // Block inputs to the game when editing input boxes
  if (ImGui::IsAnyItemActive() == true && ImGui::IsMouseDown(ImGuiMouseButton_Left) != true) return 0;
  
  // Only some movement keys
  if (event->type == SDL_KEYDOWN) {
    SDL_KeyboardEvent* key = &event->key;
    SDL_Keycode sym = key->keysym.sym;
    if (sym == SDLK_w || sym == SDLK_a || sym == SDLK_s || sym == SDLK_d || sym == SDLK_INSERT ||
	sym == SDLK_SPACE || sym == SDLK_LCTRL) {
      return 1;
    }
  }
  
  // Block everything else
  return 0;
}


bool poll_event_hook(SDL_Event* event) {
  bool ret = poll_event_original(event);
  
  if (sdl_window != nullptr && ImGui::IsImGuiFullyInitialized())
    ImGui_ImplSDL2_ProcessEvent(event);
  
  get_input(event);
  
  return ret;
}


int peep_events_hook(SDL_Event* events, int numevents, SDL_eventaction action, int min, int max) {
  int ret = peep_events_original(events, numevents, action, min, max);

  /*
  if(ret != -1 && sdl_window != nullptr && ImGui::GetCurrentContext())
    ImGui_ImplSDL2_ProcessEvent(events);

  get_input(events);
  */
  
  return ret;
}


void swap_window_hook(SDL_Window* window) {
  static SDL_GLContext original_context = NULL, new_context = NULL;

  if (!new_context) {
    original_context = SDL_GL_GetCurrentContext();
    new_context = SDL_GL_CreateContext(window);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
      print("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
      swap_window_original(window);
      return;
    }
    
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 100");
    ImGui_ImplSDL2_InitForOpenGL(window, nullptr);    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    orig_style = ImGui::GetStyle();

    set_imgui_theme();
  }

  
  SDL_GL_MakeCurrent(window, new_context);
  
  if (ImGui::IsKeyPressed(ImGuiKey_Insert, false) || ImGui::IsKeyPressed(ImGuiKey_F11, false)) {
    menu_focused = !menu_focused;
    surface->set_cursor_visible(menu_focused);
  }
  
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  draw_watermark();

  draw_tickbase_indicator();

  if (menu_focused == true) {
    draw_menu();
  }  

  
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_MakeCurrent(window, original_context);

  swap_window_original(window);
}

Uint32 get_window_flags_hook(SDL_Window* window) {
  return get_window_flags_original(window);
}

SDL_bool get_window_WM_info_hook(SDL_Window* window, SDL_SysWMinfo* info) {
  return get_window_WM_info_original(window, info);
}

// Used for grabbing the SDL_Window handle when in Vulkan mode
void get_window_size_hook(SDL_Window* window, int* w, int* h) {

  sdl_window = window;
  
  get_window_size_original(window, w, h);
}

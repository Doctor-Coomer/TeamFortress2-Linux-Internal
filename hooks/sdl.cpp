#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <GL/glew.h>

#include "../imgui/dearimgui.hpp"

#include "../interfaces/surface.hpp"

#include "../print.hpp"

#include "../gui/menu.hpp"

bool (*poll_event_original)(SDL_Event*) = NULL;
void (*swap_window_original)(void*) = NULL;
Uint32 (*get_window_flags_original)(SDL_Window*) = NULL;
SDL_bool (*get_window_WM_info_original)(SDL_Window* window, SDL_SysWMinfo* info) = NULL;
void (*get_window_size_original)(SDL_Window* window, int* w, int* h) = NULL;


bool poll_event_hook(SDL_Event* event) {
  bool ret = poll_event_original(event);

  
  if (ret == true && sdl_window != nullptr && ImGui::GetCurrentContext())
    ImGui_ImplSDL2_ProcessEvent(event);
  
  get_input(event);
  
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

    ImGuiStyle* style = &ImGui::GetStyle();

    style->Colors[ImGuiCol_WindowBg]         = ImVec4(0.1, 0.1, 0.1, 1);

    style->Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.05, 0.05, 0.05, 1);
    style->Colors[ImGuiCol_TitleBg]          = ImVec4(0.05, 0.05, 0.05, 1);

    style->Colors[ImGuiCol_CheckMark]        = ImVec4(0.869346734, 0.450980392, 0.211764706, 1);

    style->Colors[ImGuiCol_FrameBg]          = ImVec4(0.15, 0.15, 0.15, 1);
    style->Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
    style->Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

    style->Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
    style->Colors[ImGuiCol_ButtonActive]     = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

    style->Colors[ImGuiCol_SliderGrab]       = ImVec4(0.869346734, 0.450980392, 0.211764706, 1);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.899346734, 0.480980392, 0.241764706, 1);
    style->GrabMinSize = 2;

    style->Colors[ImGuiCol_Header]           = ImVec4(0.18, 0.18, 0.18, 1);
    style->Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
    style->Colors[ImGuiCol_HeaderActive]     = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

  }

  
  SDL_GL_MakeCurrent(window, new_context);
  
  if (ImGui::IsKeyPressed(ImGuiKey_Insert, false)) {
    menu_focused = !menu_focused;
    surface->set_cursor_visible(menu_focused);
  }
  
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  if (menu_focused) {
    draw_menu();
  }
  
  draw_watermark();
  
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

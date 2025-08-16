#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "../gui/imgui/dearimgui.hpp"
#include "../gui/config.hpp"

#include "../interfaces/surface.hpp"

#include "../print.hpp"

#include "../gui/menu.cpp"
#include "../hacks/navmesh/navengine.hpp"

void (*swap_window_original)(void*) = NULL;
bool (*poll_event_original)(SDL_Event*) = NULL;



bool poll_event_hook(SDL_Event* event) {
  bool ret = poll_event_original(event);

  if (ret)
    ImGui_ImplSDL2_ProcessEvent(event);
  
  get_input(event);
  
  return ret;
}

void watermark() {
  {
    ImGui::SetNextWindowPos(ImVec2(10, 10)); 
    ImGui::SetNextWindowSize(ImVec2(150, 30));
    ImGui::Begin("a", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    
    ImGui::TextCentered("I Use Arch BTW!!!");
    ImGui::End();

  }
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
  
  watermark();


  // BY SOME REASON IT CRASHES GAME EVEN WHEN ITS WRAPPED INTO 1 BAJILION NULLCHECKS...
  // bool nav_overlay_enabled = (config.nav.master && config.nav.engine_enabled && config.nav.draw_overlay);
  // nav::SetDrawEnabled(nav_overlay_enabled);
  // if (nav_overlay_enabled) {
  //   nav::Draw();
  // }
  
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_MakeCurrent(window, original_context);

  swap_window_original(window);
}

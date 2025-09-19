#ifndef DEARIMGUI_HPP
#define DEARIMGUI_HPP

#include <SDL2/SDL.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <SDL2/SDL_scancode.h>
#include <string>

static ImGuiStyle orig_style; // Defined in /hooks/sdl.cpp swap_window_hook

namespace ImGui {
  static void TextCentered(std::string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text(text.c_str());
  }

  static void SliderFloatHeightPad(const char* label, float* v, float v_min, float v_max, float height, const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
    int orig_x = orig_style.FramePadding.x;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(orig_x, height));
    ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
    ImGui::PopStyleVar(1);
  }

  
  
  static std::string GetKeyName(SDL_Scancode key) {
    if (key >= 0) {
      const char* name = SDL_GetKeyName(SDL_GetKeyFromScancode(key));
      return name ? std::string(name) : "Unknown";
    }

    switch (-key) {
    case SDL_BUTTON_LEFT:   return "Mouse Left";
    case SDL_BUTTON_RIGHT:  return "Mouse Right";
    case SDL_BUTTON_MIDDLE: return "Mouse Middle";
    case SDL_BUTTON_X1:     return "Mouse X1";
    case SDL_BUTTON_X2:     return "Mouse X2";
    default:                return "Mouse Button " + std::to_string(-key);
    }
  }

  static void KeybindBox(bool* waitingFlag, int* keycode) {
    std::string buttonLabel;
    if (*waitingFlag)
      buttonLabel = "...";
    else if (*keycode == SDLK_UNKNOWN)
      buttonLabel = " ";
    else
      buttonLabel = GetKeyName((SDL_Scancode)*keycode);

    if (ImGui::Button(buttonLabel.c_str(), ImVec2(90, 20))) {
      *waitingFlag = true;
    }
  }

  static void KeybindEvent(const SDL_Event* event, bool* waitingFlag, int* keycode) {
    if (!*waitingFlag)
      return;

    if (event->type == SDL_KEYDOWN && event->key.repeat == 0) {
      if (event->key.keysym.sym == SDLK_ESCAPE) {
	*waitingFlag = false; // cancel binding
      } else {
	*keycode = event->key.keysym.scancode;
	*waitingFlag = false;
      }
    } else if (event->type == SDL_MOUSEBUTTONDOWN) {
      *keycode = -event->button.button;
      *waitingFlag = false;
    }
  }

}


#endif

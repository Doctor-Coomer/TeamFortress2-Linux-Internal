#include "navengine.hpp"

#include <unistd.h>
#include <limits.h>
#include <string>

#include "navparser.hpp"
#include "../../gui/imgui/dearimgui.hpp"
#include "../../interfaces/debug_overlay.hpp"
#include "../../interfaces/engine.hpp"
#include "../../vec.hpp"

namespace nav {

static bool g_draw_enabled = false;
static std::string g_last_error;
static std::string g_last_map;

static std::string Dirname(const std::string &path) {
  size_t pos = path.find_last_of('/');
  if (pos == std::string::npos) return std::string(".");
  return path.substr(0, pos);
}

void SetDrawEnabled(bool enabled) { g_draw_enabled = enabled; }
bool IsDrawEnabled() { return g_draw_enabled; }

const char *GetLastError() { return g_last_error.empty() ? nullptr : g_last_error.c_str(); }

std::string ExtractMapNameFromPath(const char* level_path) {
  if (!level_path) return {};
  std::string s(level_path);
  size_t pos = s.find_last_of('/');
  if (pos != std::string::npos) s = s.substr(pos + 1);
  pos = s.find_last_of('\\');
  if (pos != std::string::npos) s = s.substr(pos + 1);
  if (s.rfind("maps/", 0) == 0) s = s.substr(5);
  if (s.size() > 4 && s.compare(s.size() - 4, 4, ".bsp") == 0) s.resize(s.size() - 4);
  return s;
}

bool EnsureLoadedForCurrentLevel() {
  if (!engine || !engine->is_in_game()) return false;
  const char* level = engine->get_level_name();
  std::string map = ExtractMapNameFromPath(level);
  if (map.empty()) return false;
  if (!IsLoaded() || map != g_last_map) {
    return LoadForMapName(map);
  }
  return true;
}

bool LoadForMapName(const std::string &map_name) {
  g_last_error.clear();
  if (map_name.empty()) {
    g_last_error = "Empty map name";
    return false;
  }
  
  std::string path = std::string("./tf/maps/") + map_name + ".nav";
  if (!Load(path)) {
    const char *err = GetError();
    g_last_error = err ? err : "Unknown error";
    return false;
  }
  g_last_map = map_name;
  return true;
}

void Draw() {
  if (!g_draw_enabled) return;
  (void)EnsureLoadedForCurrentLevel();

  ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.35f);
  if (ImGui::Begin("nav overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
    if (IsLoaded()) {
      const Info &info = GetInfo();
      ImGui::Text("navmesh loaded");
      if (!g_last_map.empty()) ImGui::Text("Map: %s", g_last_map.c_str());
      ImGui::Text("Path: %s", info.path.c_str());
      ImGui::Text("Size: %zu", info.size);
      ImGui::Text("Magic: %s", info.magic_ascii);
      ImGui::Text("Version: %d", info.version);
      ImGui::Separator();
      ImGui::Text("Places: %zu", GetPlaceCount());
      ImGui::Text("Areas: %zu", GetAreaCount());
      // ImGui::Text("Ladders: %zu", GetLadderCount()); no ladders in tf2 xd
    } else {
      ImGui::Text("No navmesh loaded");
      const char *err = GetLastError();
      if (!err) err = GetError();
      if (err) ImGui::TextColored(ImVec4(1,0.4f,0.2f,1), "Error: %s", err);
    }
  }
  ImGui::End();

}

} // namespace

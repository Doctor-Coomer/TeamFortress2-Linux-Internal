// melody 17/aug/2025

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace nav {
void SetDrawEnabled(bool enabled);
bool IsDrawEnabled();
bool LoadForMapName(const std::string &map_name);
void Draw();
const char *GetLastError();

std::string ExtractMapNameFromPath(const char* level_path);
bool EnsureLoadedForCurrentLevel();

void Visualizer_ClearPath();
void Visualizer_SetPath(const std::vector<uint32_t>& area_ids,
                        size_t next_index,
                        uint32_t goal_area_id);
void Visualizer_GetPath(const std::vector<uint32_t>** out_area_ids,
                        size_t* out_next_index,
                        uint32_t* out_goal_area_id);

}

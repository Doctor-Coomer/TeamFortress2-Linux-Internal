// melody 17/aug/2025

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../../vec.hpp"

class Player;
struct user_cmd;

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

struct CreateMoveResult {
  bool move_set = false;
  Vec3 orig_view = {};
  float orig_forward = 0.0f;
  float orig_side = 0.0f;
  bool look_applied = false;
};

CreateMoveResult OnCreateMove(Player* localplayer, user_cmd* user_cmd);

}

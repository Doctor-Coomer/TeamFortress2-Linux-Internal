// melody 17/aug/2025

#pragma once

#include <cstdint>
#include <cstddef>

namespace nav { namespace navbot {

enum class TaskKind { Roam, Snipe, ChaseMelee, GetHealth, Retreat };

struct BotContext {
  float me[3] = {0.f, 0.f, 0.f};
  int   cmd_number = 0;

  bool  have_target = false;
  int   target_index = 0;
  float target_pos[3] = {0.f, 0.f, 0.f};

  bool  scheduler_enabled = true;
  bool  snipe_enabled = true;
  float snipe_preferred_range = 900.0f;
  int   snipe_repath_ticks = 12;
  float snipe_replan_move_threshold = 96.0f;
  bool  chase_enabled = true;
  bool  chase_only_when_melee = true;
  bool  melee_equipped = false;
  float chase_distance_max = 1500.0f;
  int   chase_repath_ticks = 10;
  float chase_replan_move_threshold = 96.0f;
};

struct BotOutput {
  bool have_waypoint = false;
  float waypoint[3] = {0.f, 0.f, 0.f};
  uint32_t goal_area_id = 0;
  size_t next_index = 0;
  bool perform_crouch_jump = false;
  TaskKind task = TaskKind::Roam;
};

void Reset();
bool Tick(const BotContext& ctx, BotOutput* out);
bool PlanToPositionFrom(float me_x, float me_y, float me_z,
                        float goal_x, float goal_y, float goal_z,
                        int cmd_number);
bool PlanToPositionFrom(float me_x, float me_y, float me_z,
                        float goal_x, float goal_y, float goal_z,
                        int cmd_number);

uint32_t CurrentGoalArea();
void ClearGoal();

}} // namespace nav::navbot

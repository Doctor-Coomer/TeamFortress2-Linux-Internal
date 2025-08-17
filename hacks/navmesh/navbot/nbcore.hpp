#pragma once

#include <cstdint>
#include <cstddef>

namespace nav { namespace navbot {

struct RoamOutput {
  bool have_waypoint = false;
  float waypoint[3] = {0.f, 0.f, 0.f};
  uint32_t goal_area_id = 0;
  size_t next_index = 0;
  bool perform_crouch_jump = false;
};

void Reset();

bool Tick(float me_x, float me_y, float me_z, int cmd_number, RoamOutput* out);

}} // namespace nav::navbot

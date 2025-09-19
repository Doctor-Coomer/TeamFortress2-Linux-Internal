#ifndef TEAM_OBJECTIVE_RESOURCE_HPP
#define TEAM_OBJECTIVE_RESOURCE_HPP

#include "entity.hpp"

#include "../print.hpp"

#define MAX_CONTROL_POINTS 8

class TeamObjectiveResource {
public:

  // Control Point methods
  int get_num_control_points(void) {
    return *(int*)(this + 0x7B8);
  }

  int get_owning_team(int index) {
    return (((int*)(this + 0x1AE4)))[index];
  }

  bool is_locked(int index) {
    return ((bool*)(this + 0x1914))[index];
    //return ((bool*)(this + 0x18F4))[index];
  }

  bool can_team_capture(int index, enum tf_team team) {
    int array_index = index + (team * MAX_CONTROL_POINTS);
    return ((bool*)(this + 0xF74))[array_index];
  }

  Vec3 get_origin(int index) {
    //return (((Vec3*)(this + 0xC))[index]);
    //print("%p\n", (this + 0x7CC));
    return ((Vec3*)(this + 0x7CC))[index];
  }
};

#endif

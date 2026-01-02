#ifndef NAVMESH_HPP
#define NAVMESH_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <array>
#include <cstdint>

#include "../../math.hpp"

#include "../../vec.hpp"

#define kVisitedCap 512
#define kJumpHeight 72.0f
#define kStepHeight 18.0f
#define kHullHeight 83.0f
#define kHullWidth  49.0f
#define kZSlop      18.0f

enum TFNavAttributeType : uint32_t {
  TF_NAV_INVALID = 0x00000000u,
  TF_NAV_BLOCKED = 0x00000001u,
  TF_NAV_SPAWN_ROOM_RED  = 0x00000002u,
  TF_NAV_SPAWN_ROOM_BLUE = 0x00000004u,
  TF_NAV_SPAWN_ROOM_EXIT = 0x00000008u,

  TF_NAV_BLUE_SETUP_GATE = 0x00000800u,
  TF_NAV_RED_SETUP_GATE  = 0x00001000u,
  TF_NAV_BLOCKED_AFTER_POINT_CAPTURE = 0x00002000u,
  TF_NAV_BLOCKED_UNTIL_POINT_CAPTURE = 0x00004000u,
  TF_NAV_BLUE_ONE_WAY_DOOR = 0x00008000u,
  TF_NAV_RED_ONE_WAY_DOOR  = 0x00010000u,

  TF_NAV_NO_SPAWNING = 0x02000000u,
};
static constexpr uint32_t NAV_MESH_NAV_BLOCKER = 0x80000000u;

static constexpr uint32_t kTFBadGoalMask =
  TF_NAV_BLOCKED |
  TF_NAV_SPAWN_ROOM_RED |
  TF_NAV_SPAWN_ROOM_BLUE |
  TF_NAV_BLUE_SETUP_GATE |
  TF_NAV_RED_SETUP_GATE |
  TF_NAV_BLOCKED_AFTER_POINT_CAPTURE |
  TF_NAV_BLOCKED_UNTIL_POINT_CAPTURE |
  TF_NAV_BLUE_ONE_WAY_DOOR |
  TF_NAV_RED_ONE_WAY_DOOR;

enum class Dir : uint8_t { North=0, East=1, South=2, West=3 };

struct HidingSpot {
  uint32_t id;
  float pos[3];
  uint8_t attrs;
};

struct AreaBind { uint32_t target_area_id; uint8_t flags; };

struct Area {
  uint32_t id;
  uint32_t attributes;
  float nw[3];
  float se[3];
  float ne_z;
  float sw_z;
  std::array<std::vector<uint32_t>,4> connections; // NESW
  std::vector<HidingSpot> hiding_spots;
  uint16_t place_id;
  std::array<std::vector<uint32_t>,2> ladder_ids; // up, down
  float earliest_occupy[2];
  float light_intensity[4];
  std::vector<AreaBind> binds;
  uint32_t inherit_vis_from = 0;
  uint32_t tf_attribute_flags = 0;

  inline Vec3 center() {
    Vec3 out;
    out.x = 0.5f * (this->nw[0] + this->se[0]);
    out.y = 0.5f * (this->nw[1] + this->se[1]);
    out.z = 0.25f * (this->nw[2] + this->se[2] + this->ne_z + this->sw_z);
    return out;
  }

  inline void min_max_z(float* min_z, float* max_z) {
    float z0 = this->nw[2];
    float z1 = this->se[2];
    float z2 = this->ne_z;
    float z3 = this->sw_z;
    *min_z = std::min(std::min(z0, z1), std::min(z2, z3));
    *max_z = std::max(std::max(z0, z1), std::max(z2, z3));
  }

  inline float sample_z_at_xy(float x, float y) {
    float min_x = std::min(this->nw[0], this->se[0]);
    float max_x = std::max(this->nw[0], this->se[0]);
    float min_y = std::min(this->nw[1], this->se[1]);
    float max_y = std::max(this->nw[1], this->se[1]);
    float tx = (max_x > min_x) ? (clampf(x, min_x, max_x) - min_x) / (max_x - min_x) : 0.5f;
    float ty = (max_y > min_y) ? (clampf(y, min_y, max_y) - min_y) / (max_y - min_y) : 0.5f;
    // Corner Z mapping: SW(min_x,min_y)=sw_z, SE(max_x,min_y)=se.z, NW(min_x,max_y)=nw.z, NE(max_x,max_y)=ne_z
    float sw = this->sw_z;
    float se = this->se[2];
    float nw = this->nw[2];
    float ne = this->ne_z;
    float z_min_y = sw * (1.0f - tx) + se * tx; // along x at y=min_y
    float z_max_y = nw * (1.0f - tx) + ne * tx; // along x at y=max_y
    return z_min_y * (1.0f - ty) + z_max_y * ty;
  }

  
  inline Vec3 closest_point_to_target(Vec3 target_xy) {
    float min_x = std::min(this->nw[0], this->se[0]);
    float max_x = std::max(this->nw[0], this->se[0]);
    float min_y = std::min(this->nw[1], this->se[1]);
    float max_y = std::max(this->nw[1], this->se[1]);

    Vec3 pt{ clampf(target_xy.x, min_x, max_x), clampf(target_xy.y, min_y, max_y), 0.0f };
    pt.z = this->sample_z_at_xy(pt.x, pt.y);
    return pt;
  }

  inline bool is_disallowed_for_goal(void) {
    if ((this->tf_attribute_flags & kTFBadGoalMask) != 0) return true;
    if ((this->attributes & NAV_MESH_NAV_BLOCKER) != 0) return true;
    return false;
  }

};

struct Ladder {
  uint32_t id;
  float width;
  float top[3];
  float bottom[3];
  float length;
  uint32_t direction;
  uint8_t dangling;
  uint32_t top_forward_area;
  uint32_t top_left_area;
  uint32_t top_right_area;
  uint32_t top_behind_area;
  uint32_t bottom_area;
};

struct Path {  
  uint32_t goal_id = 0;
  std::vector<uint32_t> path_ids = {};
  size_t next_index = 0;  

  std::deque<uint32_t> visited_fifo;
  std::unordered_set<uint32_t> visited_set;
  void visited_add(uint32_t id) {
    if (!id) return;
    if (this->visited_set.find(id) != this->visited_set.end()) return;
    if (this->visited_fifo.size() >= kVisitedCap) {
      uint32_t old = this->visited_fifo.front();
      this->visited_fifo.pop_front();
      this->visited_set.erase(old);
    }
    this->visited_fifo.push_back(id);
    this->visited_set.insert(id);
  }
  
  uint32_t last_area_id = 0;
  
} static path;

struct Mesh {
  uint32_t version = 0;
  uint32_t sub_version = 0;
  uint32_t save_bsp_size = 0;
  uint8_t analyzed = 0;
  std::vector<std::string> places;
  bool has_unnamed_areas = false;
  std::vector<Area> areas;
  std::unordered_map<uint32_t, size_t> area_index_by_id; // id -> index in areas
  std::string map_name = "";
  std::vector<Ladder> ladders;

  Area* id_to_area(uint32_t id) {
    auto it = this->area_index_by_id.find(id);
    if (it == this->area_index_by_id.end()) return nullptr;
    unsigned int index = it->second;
    if (index >= this->areas.size()) return nullptr;
    return &this->areas[index];
  }

  Area* best_area_from_xyz(Vec3 start,
			   float jump_height = kJumpHeight, float z_slop = kZSlop) {
    Area* best_in = nullptr;
    float best_in_dz = std::numeric_limits<float>::max();
    Area* best_jump = nullptr;
    float best_jump_dz = std::numeric_limits<float>::max();
    Area* best_overlap = nullptr;
    float best_overlap_dz = std::numeric_limits<float>::max();

    for (Area &a : this->areas) {
      float min_x = std::min(a.nw[0], a.se[0]);
      float max_x = std::max(a.nw[0], a.se[0]);
      float min_y = std::min(a.nw[1], a.se[1]);
      float max_y = std::max(a.nw[1], a.se[1]);
      if (!(start.x >= min_x && start.x <= max_x && start.y >= min_y && start.y <= max_y))
	continue;

      float z0 = a.nw[2];
      float z1 = a.se[2];
      float z2 = a.ne_z;
      float z3 = a.sw_z;
      float min_z = std::min(std::min(z0, z1), std::min(z2, z3));
      float max_z = std::max(std::max(z0, z1), std::max(z2, z3));

      if (start.z >= (min_z - z_slop) && start.z <= (max_z + z_slop)) {
	float clamped = std::max(min_z, std::min(max_z, start.z));
	float dz = std::fabs(start.z - clamped);
	if (dz < best_in_dz) { best_in_dz = dz; best_in = &a; }
	continue;
      }

      float zj = start.z + jump_height;
      if (zj >= (min_z - z_slop) && zj <= (max_z + z_slop)) {
	float clampedj = std::max(min_z, std::min(max_z, zj));
	float dzj = std::fabs(zj - clampedj);
	if (dzj < best_jump_dz) { best_jump_dz = dzj; best_jump = &a; }
      }

      float dz_ovr = (start.z < min_z) ? (min_z - start.z) : (start.z > max_z ? (start.z - max_z) : 0.0f);
      if (dz_ovr < best_overlap_dz) { best_overlap_dz = dz_ovr; best_overlap = &a; }
    }

    if (best_in) return best_in;
    if (best_jump) return best_jump;
    return best_overlap;
  }  

  Area* find_nearest_area_2d(Vec3 start) {
    if (this->areas.empty()) return nullptr;
    Area* best = nullptr;
    float best_d2 = std::numeric_limits<float>::max();
    for (auto& a : this->areas) {
      if (a.is_disallowed_for_goal()) continue;
      Vec3 c = a.center();
      // Height reachability: skip areas whose floor is too far above us to climb/jump.
      float z0 = a.nw[2], z1 = a.se[2], z2 = a.ne_z, z3 = a.sw_z;
      float min_z = std::min(std::min(z0, z1), std::min(z2, z3));
      if ((min_z - start.z) > (kJumpHeight + kZSlop)) continue;
      float d2 = distance_squared_2d(start, c);
      if (d2 < best_d2) { best_d2 = d2; best = &a; }
    }
    return best;
  }
  
  uint32_t pick_far_goal_from_here(Area* start, float me_x, float me_y, float me_z) {
    if (start == nullptr) return 0;
    
    Vec3 me2 = {me_x, me_y, me_z};
    uint32_t best_nonvisited_id = 0;
    float best_nonvisited_d2 = -1.f;
    uint32_t best_any_id = 0;
    float best_any_d2 = -1.f;
    for (Area &a : this->areas) {
      if (a.id == start->id) continue;
      if (a.is_disallowed_for_goal()) continue;
      Vec3 c = a.center();
      float d2 = distance_squared_2d(me2, c);
      if (d2 > best_any_d2) { best_any_d2 = d2; best_any_id = a.id; }
      if (path.visited_set.find(a.id) == path.visited_set.end()) {
	if (d2 > best_nonvisited_d2) { best_nonvisited_d2 = d2; best_nonvisited_id = a.id; }
      }
    }
    return best_nonvisited_id ? best_nonvisited_id : best_any_id;
  }

} static mesh;

#endif

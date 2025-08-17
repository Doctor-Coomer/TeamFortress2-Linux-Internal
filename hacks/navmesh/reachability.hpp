// melody 17/aug/2025
#pragma once

#include <cmath>
#include <algorithm>

#include "navparser.hpp"
#include "../../vec.hpp"
#include "../../interfaces/engine_trace.hpp"

namespace nav {
namespace reach {

static constexpr float kHullWidth = 49.0f;
static constexpr float kHullHalfWidth = kHullWidth * 0.5f;
static constexpr float kHullHeight = 83.0f;
static constexpr float kJumpHeight = 72.0f;
static constexpr float kZSlop = 18.0f;

inline void ComputeAreaCenterVec(const Area* a, Vec3& out) {
  out.x = 0.5f * (a->nw[0] + a->se[0]);
  out.y = 0.5f * (a->nw[1] + a->se[1]);
  out.z = 0.25f * (a->nw[2] + a->se[2] + a->ne_z + a->sw_z);
}

inline void ComputeAreaMinMaxZ(const Area* a, float& min_z, float& max_z) {
  float z0 = a->nw[2];
  float z1 = a->se[2];
  float z2 = a->ne_z;
  float z3 = a->sw_z;
  min_z = std::min(std::min(z0, z1), std::min(z2, z3));
  max_z = std::max(std::max(z0, z1), std::max(z2, z3));
}

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

inline float SampleAreaZAtXY(const Area* a, float x, float y) {
  float min_x = std::min(a->nw[0], a->se[0]);
  float max_x = std::max(a->nw[0], a->se[0]);
  float min_y = std::min(a->nw[1], a->se[1]);
  float max_y = std::max(a->nw[1], a->se[1]);
  float tx = (max_x > min_x) ? (clampf(x, min_x, max_x) - min_x) / (max_x - min_x) : 0.5f;
  float ty = (max_y > min_y) ? (clampf(y, min_y, max_y) - min_y) / (max_y - min_y) : 0.5f;
  // Corner Z mapping: SW(min_x,min_y)=sw_z, SE(max_x,min_y)=se.z, NW(min_x,max_y)=nw.z, NE(max_x,max_y)=ne_z
  float sw = a->sw_z;
  float se = a->se[2];
  float nw = a->nw[2];
  float ne = a->ne_z;
  float z_min_y = sw * (1.0f - tx) + se * tx; // along x at y=min_y
  float z_max_y = nw * (1.0f - tx) + ne * tx; // along x at y=max_y
  return z_min_y * (1.0f - ty) + z_max_y * ty;
}

inline Vec3 ClosestPointOnAreaToward(const Area* a, const Vec3& target_xy) {
  float min_x = std::min(a->nw[0], a->se[0]);
  float max_x = std::max(a->nw[0], a->se[0]);
  float min_y = std::min(a->nw[1], a->se[1]);
  float max_y = std::max(a->nw[1], a->se[1]);
  Vec3 pt{ clampf(target_xy.x, min_x, max_x), clampf(target_xy.y, min_y, max_y), 0.0f };
  pt.z = SampleAreaZAtXY(a, pt.x, pt.y);
  return pt;
}

// Dropdown handling: adjust a midpoint to avoid edge-sticking when moving up/down between areas.
inline Vec3 AdjustForDropdown(const Vec3& current_pos, const Vec3& next_pos) {
  Vec3 to_target{ next_pos.x - current_pos.x, next_pos.y - current_pos.y, next_pos.z - current_pos.z };
  float height_diff = to_target.z;

  // Zero-out Z for directional movement on XY plane
  to_target.z = 0.0f;
  float len_xy = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);
  if (len_xy > 0.0001f) { to_target.x /= len_xy; to_target.y /= len_xy; }

  if (height_diff < 0.0f) {
    float drop_distance = -height_diff;
    if (drop_distance <= kJumpHeight) {
      return current_pos; // small drop, no special handling
    }
    float push = (drop_distance <= kHullHeight) ? (kHullWidth * 1.5f) : (kHullWidth * 2.5f);
    return Vec3{ current_pos.x + to_target.x * push, current_pos.y + to_target.y * push, current_pos.z };
  }

  if (height_diff > 0.0f && height_diff <= kJumpHeight) {
    float pull = kHullWidth * 0.5f;
    return Vec3{ current_pos.x - to_target.x * pull, current_pos.y - to_target.y * pull, current_pos.z };
  }

  return current_pos;
}

inline bool IsPassableSegment(const Vec3& from, const Vec3& to,
                              float lift_z = 0.0f,
                              void* skip_entity = nullptr,
                              unsigned int mask = MASK_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MOVEABLE | CONTENTS_GRATE,
                              float fraction_threshold = 0.98f) {
  if (!engine_trace) return true;

  Vec3 mins{ -kHullHalfWidth, -kHullHalfWidth, 0.0f };
  Vec3 maxs{  kHullHalfWidth,  kHullHalfWidth, kHullHeight };
  Vec3 f = from; Vec3 t = to; f.z += lift_z; t.z += lift_z;

  ray_t ray = engine_trace->init_ray(&f, &t, &mins, &maxs);
  trace_filter filter; engine_trace->init_trace_filter(&filter, skip_entity);
  trace_t tr{};
  engine_trace->trace_ray(&ray, mask, &filter, &tr);

  return (!tr.all_solid && !tr.start_solid && tr.fraction > fraction_threshold);
}

} // namespace reach
} // namespace nav

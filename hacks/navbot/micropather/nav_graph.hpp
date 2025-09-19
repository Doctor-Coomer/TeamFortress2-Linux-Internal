#ifndef NAV_GRAPH_HPP
#define NAV_GRAPH_HPP

#include "micropather.h"

#include "../navmesh.hpp"

class NavGraph : public micropather::Graph {
public:
  float LeastCostEstimate(void* stateStart, void* stateEnd) override {
    Area* a = static_cast<Area*>(stateStart);
    Area* b = static_cast<Area*>(stateEnd);
    Vec3 ca, cb;
    ca = a->center();
    cb = b->center();
    return distance_3d(ca, cb);
  }

  void AdjacentCost(void* state, MP_VECTOR<micropather::StateCost>* adjacent) override {
    adjacent->clear();
    Area* a = static_cast<Area*>(state);
    if (!a) return;

    Vec3 ca;
    ca = a->center();
    float a_min_z, a_max_z;
    a->min_max_z(&a_min_z, &a_max_z);

    for (int d = 0; d < 4; ++d) {
      const auto& ids = a->connections[d];
      for (uint32_t nid : ids) {
        Area* n = this->GetAreaByIdInternal(&mesh, nid);
        if (!n) continue;
        float n_min_z, n_max_z;
        n->min_max_z(&n_min_z, &n_max_z);
        if (n_min_z - a_max_z > (kJumpHeight + kZSlop)) {
          continue;
        }
	
        Vec3 cn = n->center();

        Vec3 center_point = a->closest_point_to_target(cn);
        Vec3 center_next = n->closest_point_to_target(center_point);

        Vec3 center_adjusted = adjust_for_dropdown(center_point, center_next);

        float height_diff = center_next.z - center_adjusted.z;
        if (height_diff > kJumpHeight) {
          continue;
        }

        Vec3 from_to_adj = { ca.x, ca.y, ca.z };
        Vec3 nextp = { center_next.x, center_next.y, center_next.z };
        float cost = distance_3d(from_to_adj, nextp);
        micropather::StateCost sc{(void*)n, cost};
        adjacent->push_back(sc);
      }
    }
  }

  void PrintStateInfo(void* state) override {
    const Area* a = static_cast<const Area*>(state);
    if (a) std::printf("Area(%u)", a->id);
    else std::printf("Area(null)");
  }

private:
  static Area* GetAreaByIdInternal(Mesh* mesh, uint32_t id) {
    auto it = mesh->area_index_by_id.find(id);
    if (it == mesh->area_index_by_id.end()) return nullptr;
    size_t idx = it->second;
    if (idx >= mesh->areas.size()) return nullptr;
    return &mesh->areas[idx];
  }

  // Dropdown handling: adjust a midpoint to avoid edge-sticking when moving up/down between areas.
  Vec3 adjust_for_dropdown(const Vec3& current_pos, const Vec3& next_pos) {
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

};

#endif

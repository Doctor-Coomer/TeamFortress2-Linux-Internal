// melody 17/aug/2025

#include "pathfinder.hpp"
#include "micropather/micropather.h"

#include <cmath>
#include <cstdio>
#include <algorithm>

namespace nav {
namespace path {

static const Mesh* s_mesh = nullptr;
static micropather::MicroPather* s_pather = nullptr;

static inline void ComputeAreaCenter(const Area* a, float out[3]) {
  out[0] = 0.5f * (a->nw[0] + a->se[0]);
  out[1] = 0.5f * (a->nw[1] + a->se[1]);
  out[2] = 0.25f * (a->nw[2] + a->se[2] + a->ne_z + a->sw_z);
}
static inline float Distance3(const float a[3], const float b[3]) {
  float dx = a[0] - b[0];
  float dy = a[1] - b[1];
  float dz = a[2] - b[2];
  return std::sqrt(dx*dx + dy*dy + dz*dz);
}

class NavGraph : public micropather::Graph {
public:
  float LeastCostEstimate(void* stateStart, void* stateEnd) override {
    const Area* a = static_cast<const Area*>(stateStart);
    const Area* b = static_cast<const Area*>(stateEnd);
    float ca[3], cb[3];
    ComputeAreaCenter(a, ca);
    ComputeAreaCenter(b, cb);
    return Distance3(ca, cb);
  }

  void AdjacentCost(void* state, MP_VECTOR<micropather::StateCost>* adjacent) override {
    adjacent->clear();
    const Area* a = static_cast<const Area*>(state);
    if (!a) return;
    const Mesh* mesh = GetMesh();
    if (!mesh) return;

    float ca[3];
    ComputeAreaCenter(a, ca);

    for (int d = 0; d < 4; ++d) {
      const auto& ids = a->connections[d];
      for (uint32_t nid : ids) {
        const Area* n = GetAreaByIdInternal(mesh, nid);
        if (!n) continue;
        float cn[3];
        ComputeAreaCenter(n, cn);
        float cost = Distance3(ca, cn);
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
  static const Area* GetAreaByIdInternal(const Mesh* mesh, uint32_t id) {
    auto it = mesh->area_index_by_id.find(id);
    if (it == mesh->area_index_by_id.end()) return nullptr;
    size_t idx = it->second;
    if (idx >= mesh->areas.size()) return nullptr;
    return &mesh->areas[idx];
  }
};

static NavGraph s_graph;

void Reset() {
  delete s_pather;
  s_pather = nullptr;
  s_mesh = nullptr;
}

bool EnsureReady() {
  if (!IsLoaded()) { Reset(); return false; }
  const Mesh* mesh = GetMesh();
  if (!mesh) { Reset(); return false; }
  if (s_pather && mesh == s_mesh) return true;

  delete s_pather;
  s_mesh = mesh;

  unsigned int alloc = 250;
  const size_t n = mesh->areas.size();
  if (n > 0) alloc = std::max<unsigned int>(250, static_cast<unsigned int>(n/4 + 16));

  s_pather = new micropather::MicroPather(&s_graph, alloc, 6, true);
  return true;
}

const Area* GetAreaById(uint32_t id) {
  const Mesh* mesh = GetMesh();
  if (!mesh) return nullptr;
  auto it = mesh->area_index_by_id.find(id);
  if (it == mesh->area_index_by_id.end()) return nullptr;
  size_t idx = it->second;
  if (idx >= mesh->areas.size()) return nullptr;
  return &mesh->areas[idx];
}

void GetAreaCenter(const Area* a, float out[3]) {
  if (!a) { out[0] = out[1] = out[2] = 0.0f; return; }
  ComputeAreaCenter(a, out);
}

int FindPath(uint32_t startAreaId, uint32_t endAreaId,
             std::vector<const Area*>& outAreas,
             float* totalCost) {
  outAreas.clear();
  if (!EnsureReady()) return micropather::MicroPather::NO_SOLUTION;

  const Area* a = GetAreaById(startAreaId);
  const Area* b = GetAreaById(endAreaId);
  if (!a || !b) return micropather::MicroPather::NO_SOLUTION;

  MP_VECTOR<void*> states;
  float cost = 0.0f;
  int rc = s_pather->Solve((void*)a, (void*)b, &states, &cost);
  if (rc == micropather::MicroPather::SOLVED || rc == micropather::MicroPather::START_END_SAME) {
    outAreas.reserve(states.size());
    for (void* s : states) {
      outAreas.push_back(static_cast<const Area*>(s));
    }
    if (totalCost) *totalCost = cost;
  }
  return rc;
}

} // namespace path
} // namespace nav


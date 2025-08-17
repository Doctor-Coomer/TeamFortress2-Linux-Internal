#pragma once

#include <cstdint>
#include <vector>

#include "navparser.hpp"

namespace nav {
namespace path {

void Reset();
bool EnsureReady();
const Area* GetAreaById(uint32_t id);
void GetAreaCenter(const Area* a, float out[3]);
int FindPath(uint32_t startAreaId, uint32_t endAreaId,
             std::vector<const Area*>& outAreas,
             float* totalCost);

} // namespace path
} // namespace nav

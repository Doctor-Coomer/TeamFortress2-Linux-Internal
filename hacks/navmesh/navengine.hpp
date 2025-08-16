#pragma once

#include <string>

namespace nav {
void SetDrawEnabled(bool enabled);
bool IsDrawEnabled();
bool LoadForMapName(const std::string &map_name);
void Draw();
const char *GetLastError();

}

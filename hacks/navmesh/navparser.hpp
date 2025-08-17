#pragma once

#include <cstdint>
#include <string>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include <array>

namespace nav {

struct Info {
  bool loaded = false;
  std::string path;
  std::string error;
  uint32_t magic_le = 0;
  char magic_ascii[5] = {0,0,0,0,0};
  int32_t version = -1;
  size_t size = 0;
};

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
  float earliest_occupy[2]{};
  float light_intensity[4]{};
  std::vector<AreaBind> binds;
  uint32_t inherit_vis_from = 0;
  uint32_t tf_attribute_flags = 0;
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

struct Mesh {
  uint32_t version = 0;
  uint32_t sub_version = 0;
  uint32_t save_bsp_size = 0;
  uint8_t analyzed = 0;
  std::vector<std::string> places;
  bool has_unnamed_areas = false;
  std::vector<Area> areas;
  std::unordered_map<uint32_t, size_t> area_index_by_id; // id -> index in areas
  std::vector<Ladder> ladders;
};

bool Load(const std::string &path);
bool IsLoaded();
const Info &GetInfo();
const char *GetError();
uint32_t GetMagicLE();
const char *GetMagicASCII();
int32_t GetVersion();
size_t GetSize();
const std::string &GetPath();
const Mesh* GetMesh();
size_t GetAreaCount();
size_t GetPlaceCount();
size_t GetLadderCount();
const Area* GetAreaByIndex(size_t i);
const Area* FindAreaContainingXY(float x, float y);
const Area* FindBestAreaAtPosition(float x, float y, float z,
                                   float jump_height = 72.0f,
                                   float z_slop = 18.0f);

} // namespace

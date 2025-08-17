#include "navparser.hpp"

#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

namespace nav {

static Info g_info;
static Mesh g_mesh;

static void Reset() {
  g_info = Info{};
  g_mesh = Mesh{};
}

bool Load(const std::string &path) {
  Reset();
  g_info.path = path;

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    g_info.error = "Failed to open file";
    return false;
  }

  // compute size
  file.seekg(0, std::ios::end);
  std::streamoff sz = file.tellg();
  if (sz < 8) { // a bit of magic needed! + version
    g_info.error = "File too small to be a .nav";
    return false;
  }
  g_info.size = static_cast<size_t>(sz);
  file.seekg(0, std::ios::beg);

  // Do You Believe In Magic? as 4 raw bytes
  unsigned char magic_bytes[4] = {0,0,0,0};
  file.read(reinterpret_cast<char*>(magic_bytes), 4);
  if (!file) {
    g_info.error = "Failed to read magic";
    return false;
  }
  g_info.magic_ascii[0] = static_cast<char>(magic_bytes[0]);
  g_info.magic_ascii[1] = static_cast<char>(magic_bytes[1]);
  g_info.magic_ascii[2] = static_cast<char>(magic_bytes[2]);
  g_info.magic_ascii[3] = static_cast<char>(magic_bytes[3]);
  g_info.magic_ascii[4] = '\0';
  g_info.magic_le = (static_cast<uint32_t>(magic_bytes[0])      ) |
                    (static_cast<uint32_t>(magic_bytes[1]) <<  8) |
                    (static_cast<uint32_t>(magic_bytes[2]) << 16) |
                    (static_cast<uint32_t>(magic_bytes[3]) << 24);

  // Accept either "NAVI" (some games) or legacy 0xFEEDFACE
  bool is_navi = (g_info.magic_ascii[0] == 'N' && g_info.magic_ascii[1] == 'A' &&
                  g_info.magic_ascii[2] == 'V' && g_info.magic_ascii[3] == 'I');
  bool is_feedface = (g_info.magic_le == 0xFEEDFACEu);
  if (!is_navi && !is_feedface) {
    g_info.error = std::string("Invalid magic (expected NAVI or FEEDFACE), got: ") + g_info.magic_ascii;
    return false;
  }

  int32_t ver = -1;
  file.read(reinterpret_cast<char*>(&ver), sizeof(ver));
  if (!file) {
    g_info.error = "Failed to read version";
    return false;
  }
  g_info.version = ver;
  g_mesh.version = static_cast<uint32_t>(ver);

  if (ver >= 10) {
    uint32_t sub = 0;
    file.read(reinterpret_cast<char*>(&sub), sizeof(sub));
    if (!file) { g_info.error = "Failed to read subVersion"; return false; }
    g_mesh.sub_version = sub;
  }
  if (ver >= 4) {
    uint32_t bsp_size = 0;
    file.read(reinterpret_cast<char*>(&bsp_size), sizeof(bsp_size));
    if (!file) { g_info.error = "Failed to read saveBspSize"; return false; }
    g_mesh.save_bsp_size = bsp_size;
  }
  if (ver >= 14) {
    uint8_t analyzed = 0;
    file.read(reinterpret_cast<char*>(&analyzed), sizeof(analyzed));
    if (!file) { g_info.error = "Failed to read isAnalyzed"; return false; }
    g_mesh.analyzed = analyzed;
  }

  // places callouts
  if (ver >= 5) {
    uint16_t place_count = 0;
    file.read(reinterpret_cast<char*>(&place_count), sizeof(place_count));
    if (!file) { g_info.error = "Failed to read places count"; return false; }
    g_mesh.places.reserve(place_count);
    for (uint16_t i = 0; i < place_count; ++i) {
      uint16_t len = 0;
      file.read(reinterpret_cast<char*>(&len), sizeof(len));
      if (!file) { g_info.error = "Failed to read place name length"; return false; }
      std::string name;
      name.resize(len);
      if (len > 0) {
        file.read(name.data(), len);
        if (!file) { g_info.error = "Failed to read place name"; return false; }
      }
      g_mesh.places.push_back(name);
    }
    if (ver > 11) {
      uint8_t has_unnamed = 0;
      file.read(reinterpret_cast<char*>(&has_unnamed), sizeof(has_unnamed));
      if (!file) { g_info.error = "Failed to read unnamed areas flag"; return false; }
      g_mesh.has_unnamed_areas = (has_unnamed != 0);
    }
  }

  // Areas
  uint32_t area_count = 0;
  file.read(reinterpret_cast<char*>(&area_count), sizeof(area_count));
  if (!file) { g_info.error = "Failed to read area count"; return false; }
  g_mesh.areas.reserve(area_count);
  for (uint32_t i = 0; i < area_count; ++i) {
    Area a{};
    file.read(reinterpret_cast<char*>(&a.id), sizeof(a.id));
    if (!file) { g_info.error = "Failed to read area id"; return false; }
    if (ver >= 13) {
      file.read(reinterpret_cast<char*>(&a.attributes), sizeof(uint32_t));
    } else if (ver >= 9) {
      uint16_t attr16 = 0; file.read(reinterpret_cast<char*>(&attr16), sizeof(attr16)); a.attributes = attr16;
    } else {
      uint8_t attr8 = 0; file.read(reinterpret_cast<char*>(&attr8), sizeof(attr8)); a.attributes = attr8;
    }
    if (!file) { g_info.error = "Failed to read area attributes"; return false; }

    // NW, SE
    file.read(reinterpret_cast<char*>(&a.nw[0]), sizeof(float)*3);
    file.read(reinterpret_cast<char*>(&a.se[0]), sizeof(float)*3);
    if (!file) { g_info.error = "Failed to read area corners"; return false; }
    // NE, SW Z
    file.read(reinterpret_cast<char*>(&a.ne_z), sizeof(float));
    file.read(reinterpret_cast<char*>(&a.sw_z), sizeof(float));
    if (!file) { g_info.error = "Failed to read NE/SW z"; return false; }

    // NESW area connections
    for (int d = 0; d < 4; ++d) {
      uint32_t cnt = 0; file.read(reinterpret_cast<char*>(&cnt), sizeof(cnt));
      if (!file) { g_info.error = "Failed to read connection count"; return false; }
      // Sanity check
      if (cnt > 1000000) { g_info.error = "Connection count unreasonable (>1e6)"; return false; }
      a.connections[d].resize(cnt);
      for (uint32_t k = 0; k < cnt; ++k) {
        file.read(reinterpret_cast<char*>(&a.connections[d][k]), sizeof(uint32_t));
        if (!file) { g_info.error = "Failed to read connection id"; return false; }
      }
    }

    // Hiding spots
    uint8_t hide_cnt = 0; file.read(reinterpret_cast<char*>(&hide_cnt), sizeof(hide_cnt));
    if (!file) { g_info.error = "Failed to read hiding spot count"; return false; }
    a.hiding_spots.reserve(hide_cnt);
    for (uint8_t h = 0; h < hide_cnt; ++h) {
      HidingSpot hs{};
      file.read(reinterpret_cast<char*>(&hs.id), sizeof(hs.id));
      file.read(reinterpret_cast<char*>(&hs.pos[0]), sizeof(float)*3);
      file.read(reinterpret_cast<char*>(&hs.attrs), sizeof(uint8_t));
      if (!file) { g_info.error = "Failed to read hiding spot"; return false; }
      a.hiding_spots.push_back(hs);
    }

    if (ver < 15) {
      uint8_t approach_cnt = 0; file.read(reinterpret_cast<char*>(&approach_cnt), sizeof(approach_cnt));
      if (!file) { g_info.error = "Failed to read approach count"; return false; }
      // Each approachSpot_t is 4+4+1+4+1 bytes;
      for (uint8_t ap = 0; ap < approach_cnt; ++ap) {
        uint32_t tmpu32; uint8_t tmpu8;
        file.read(reinterpret_cast<char*>(&tmpu32), 4);
        file.read(reinterpret_cast<char*>(&tmpu32), 4);
        file.read(reinterpret_cast<char*>(&tmpu8), 1);
        file.read(reinterpret_cast<char*>(&tmpu32), 4);
        file.read(reinterpret_cast<char*>(&tmpu8), 1);
        if (!file) { g_info.error = "Failed to skip approach spot"; return false; }
      }
    }

    uint32_t enc_cnt = 0; file.read(reinterpret_cast<char*>(&enc_cnt), sizeof(enc_cnt));
    if (!file) { g_info.error = "Failed to read encounter path count"; return false; }
    for (uint32_t e = 0; e < enc_cnt; ++e) {
      // encounterPath_t has two uints + two bytes + spots btw
      uint32_t tmpu32; uint8_t tmpu8; uint8_t spot_cnt;
      file.read(reinterpret_cast<char*>(&tmpu32), 4); // Entryareaid
      file.read(reinterpret_cast<char*>(&tmpu8), 1);  // Entrydirection
      file.read(reinterpret_cast<char*>(&tmpu32), 4); // Destareaid
      file.read(reinterpret_cast<char*>(&tmpu8), 1);  // Destdirection
      file.read(reinterpret_cast<char*>(&spot_cnt), 1);
      if (!file) { g_info.error = "Failed to read encounter path header"; return false; }
      for (uint8_t s = 0; s < spot_cnt; ++s) {
        file.read(reinterpret_cast<char*>(&tmpu32), 4); // Areaid
        file.read(reinterpret_cast<char*>(&tmpu8), 1);  // Parametricdistance
        if (!file) { g_info.error = "Failed to read encounter spot"; return false; }
      }
    }

    // placeid
    uint16_t place = 0; file.read(reinterpret_cast<char*>(&place), sizeof(place));
    if (!file) { g_info.error = "Failed to read place id"; return false; }
    a.place_id = place;

    // lodderz (ladders useless bcuz there are no ladders in tf2 but navmesh system has them so why not lol)
    for (int v = 0; v < 2; ++v) {
      uint32_t lc = 0; file.read(reinterpret_cast<char*>(&lc), sizeof(lc));
      if (!file) { g_info.error = "Failed to read ladder id count"; return false; }
      a.ladder_ids[v].resize(lc);
      for (uint32_t li = 0; li < lc; ++li) {
        file.read(reinterpret_cast<char*>(&a.ladder_ids[v][li]), sizeof(uint32_t));
        if (!file) { g_info.error = "Failed to read ladder id"; return false; }
      }
    }

    //earliest occupy times (2 floats), light intenstties (4 floats)
    file.read(reinterpret_cast<char*>(&a.earliest_occupy[0]), sizeof(float)*2);
    file.read(reinterpret_cast<char*>(&a.light_intensity[0]), sizeof(float)*4);
    if (!file) { g_info.error = "Failed to read area timing/light"; return false; }

    // area binds
    uint32_t bind_cnt = 0; file.read(reinterpret_cast<char*>(&bind_cnt), sizeof(bind_cnt));
    if (!file) { g_info.error = "Failed to read area bind count"; return false; }
    a.binds.reserve(bind_cnt);
    for (uint32_t bi = 0; bi < bind_cnt; ++bi) {
      AreaBind b{};
      file.read(reinterpret_cast<char*>(&b.target_area_id), sizeof(uint32_t));
      file.read(reinterpret_cast<char*>(&b.flags), sizeof(uint8_t));
      if (!file) { g_info.error = "Failed to read area bind"; return false; }
      a.binds.push_back(b);
    }

    // inherit vis from area id
    file.read(reinterpret_cast<char*>(&a.inherit_vis_from), sizeof(uint32_t));
    if (!file) { g_info.error = "Failed to read inherit visibility"; return false; }

    // area flags (navmesh 16)
    if (g_mesh.version >= 16 && g_mesh.sub_version == 2) {
      file.read(reinterpret_cast<char*>(&a.tf_attribute_flags), sizeof(uint32_t));
      if (!file) { g_info.error = "Failed to read TF2 attribute flags"; return false; }
    }

    g_mesh.area_index_by_id[a.id] = g_mesh.areas.size();
    g_mesh.areas.push_back(std::move(a));
  }

  // Ladders
  if (ver >= 6) {
    uint32_t ladder_count = 0; file.read(reinterpret_cast<char*>(&ladder_count), sizeof(ladder_count));
    if (!file) { g_info.error = "Failed to read ladder count"; return false; }
    g_mesh.ladders.reserve(ladder_count);
    for (uint32_t li = 0; li < ladder_count; ++li) {
      Ladder l{};
      file.read(reinterpret_cast<char*>(&l.id), sizeof(l.id));
      file.read(reinterpret_cast<char*>(&l.width), sizeof(l.width));
      file.read(reinterpret_cast<char*>(&l.top[0]), sizeof(float)*3);
      file.read(reinterpret_cast<char*>(&l.bottom[0]), sizeof(float)*3);
      file.read(reinterpret_cast<char*>(&l.length), sizeof(l.length));
      file.read(reinterpret_cast<char*>(&l.direction), sizeof(l.direction));
      if (ver == 6) {
        file.read(reinterpret_cast<char*>(&l.dangling), sizeof(uint8_t));
      } else {
        l.dangling = 0;
      }
      file.read(reinterpret_cast<char*>(&l.top_forward_area), sizeof(uint32_t));
      file.read(reinterpret_cast<char*>(&l.top_left_area), sizeof(uint32_t));
      file.read(reinterpret_cast<char*>(&l.top_right_area), sizeof(uint32_t));
      file.read(reinterpret_cast<char*>(&l.top_behind_area), sizeof(uint32_t));
      file.read(reinterpret_cast<char*>(&l.bottom_area), sizeof(uint32_t));
      if (!file) { g_info.error = "Failed to read ladder"; return false; }
      g_mesh.ladders.push_back(l);
    }
  }

  g_info.loaded = true;
  return true;
}

bool IsLoaded() { return g_info.loaded; }
const Info &GetInfo() { return g_info; }
const char *GetError() { return g_info.error.empty() ? nullptr : g_info.error.c_str(); }
uint32_t GetMagicLE() { return g_info.magic_le; }
const char *GetMagicASCII() { return g_info.magic_ascii; }
int32_t GetVersion() { return g_info.version; }
size_t GetSize() { return g_info.size; }
const std::string &GetPath() { return g_info.path; }

const Mesh* GetMesh() { return g_info.loaded ? &g_mesh : nullptr; }
size_t GetAreaCount() { return g_info.loaded ? g_mesh.areas.size() : 0; }
size_t GetPlaceCount() { return g_info.loaded ? g_mesh.places.size() : 0; }
size_t GetLadderCount() { return g_info.loaded ? g_mesh.ladders.size() : 0; }
const Area* GetAreaByIndex(size_t i) {
  if (!g_info.loaded) return nullptr;
  if (i >= g_mesh.areas.size()) return nullptr;
  return &g_mesh.areas[i];
}

const Area* FindAreaContainingXY(float x, float y) {
  if (!g_info.loaded) return nullptr;
  for (const Area &a : g_mesh.areas) {
    float min_x = std::min(a.nw[0], a.se[0]);
    float max_x = std::max(a.nw[0], a.se[0]);
    float min_y = std::min(a.nw[1], a.se[1]);
    float max_y = std::max(a.nw[1], a.se[1]);
    if (x >= min_x && x <= max_x && y >= min_y && y <= max_y) {
      return &a;
    }
  }
  return nullptr;
}

const Area* FindBestAreaAtPosition(float x, float y, float z,
                                   float jump_height,
                                   float z_slop) {
  if (!g_info.loaded) return nullptr;

  const Area* best_in = nullptr;
  float best_in_dz = std::numeric_limits<float>::max();
  const Area* best_jump = nullptr;
  float best_jump_dz = std::numeric_limits<float>::max();
  const Area* best_overlap = nullptr;
  float best_overlap_dz = std::numeric_limits<float>::max();

  for (const Area &a : g_mesh.areas) {
    float min_x = std::min(a.nw[0], a.se[0]);
    float max_x = std::max(a.nw[0], a.se[0]);
    float min_y = std::min(a.nw[1], a.se[1]);
    float max_y = std::max(a.nw[1], a.se[1]);
    if (!(x >= min_x && x <= max_x && y >= min_y && y <= max_y))
      continue;

    float z0 = a.nw[2];
    float z1 = a.se[2];
    float z2 = a.ne_z;
    float z3 = a.sw_z;
    float min_z = std::min(std::min(z0, z1), std::min(z2, z3));
    float max_z = std::max(std::max(z0, z1), std::max(z2, z3));

    if (z >= (min_z - z_slop) && z <= (max_z + z_slop)) {
      float clamped = std::max(min_z, std::min(max_z, z));
      float dz = std::fabs(z - clamped);
      if (dz < best_in_dz) { best_in_dz = dz; best_in = &a; }
      continue;
    }

    float zj = z + jump_height;
    if (zj >= (min_z - z_slop) && zj <= (max_z + z_slop)) {
      float clampedj = std::max(min_z, std::min(max_z, zj));
      float dzj = std::fabs(zj - clampedj);
      if (dzj < best_jump_dz) { best_jump_dz = dzj; best_jump = &a; }
    }

    float dz_ovr = (z < min_z) ? (min_z - z) : (z > max_z ? (z - max_z) : 0.0f);
    if (dz_ovr < best_overlap_dz) { best_overlap_dz = dz_ovr; best_overlap = &a; }
  }

  if (best_in) return best_in;
  if (best_jump) return best_jump;
  return best_overlap;
}

} // namespace

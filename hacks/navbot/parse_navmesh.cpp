#include "navmesh.hpp"

#include <fstream>
#include <cstdint>

#include "../../gui/config.hpp"

#include "../../print.hpp"

std::string level_path_to_map_name(const char* level_path) {
  if (level_path == nullptr) return "";

  // const char* to std::string
  std::string map_name = std::string(level_path);

  // parse file name / map name
  map_name = map_name.substr(map_name.find_last_of('/')+1);
  map_name.resize(map_name.find_last_of('.'));    

  return map_name;
}


void parse_navmesh(const char* level_path) {
  if (config.navbot.master == false) {
    mesh = {};
    return;
  }
  
  std::string map_name = level_path_to_map_name(level_path);
  if (map_name == "") return;
  
  if (mesh.map_name != map_name)
    mesh = {};
  else if (mesh.map_name == map_name)
    return;
  
  std::string nav_file_path = ("./tf/maps/" + map_name + ".nav");

  std::ifstream nav_file = std::ifstream(nav_file_path, std::ios::binary);
  if (!nav_file.is_open()) {
    print("%s.nav is missing.\n", map_name.c_str());
    nav_file.close();
    return;
  }
  
  nav_file.seekg(0, std::ios::end);
  std::streamoff sz = nav_file.tellg();
  if (sz < 8) { // magic + version
    nav_file.close();
    return;
  }
  nav_file.seekg(0, std::ios::beg);

  
  // Do You Believe In Magic? as 4 raw bytes
  unsigned char magic_bytes[4] = {0,0,0,0};
  nav_file.read((char*)(magic_bytes), 4);

  unsigned int magic_bytes_le =
    ((uint32_t)magic_bytes[0])       |
    ((uint32_t)magic_bytes[1] << 8)  |
    ((uint32_t)magic_bytes[2] << 16) |
    ((uint32_t)magic_bytes[3] << 24);

  // NAVI and 0xFEEDFACE are both of the supported .nav formats
  if (!(magic_bytes[0] == 'N' && magic_bytes[1] == 'A' && magic_bytes[2] == 'V' && magic_bytes[3] == 'I') &&
      !(magic_bytes_le == 0xFEEDFACEu))
    {
      print("%s.nav is not a nav file. Invalid magic number.\n", map_name.c_str());
      nav_file.close();
      return;
    }

  // Add the map name now because we've verified that this is a valid .nav
  mesh.map_name = map_name;

  
  int32_t ver = -1;
  nav_file.read((char*)(&ver), sizeof(ver));
  mesh.version = static_cast<uint32_t>(ver);

  
  if (ver >= 10) {
    uint32_t sub = 0;
    nav_file.read((char*)(&sub), sizeof(sub));
    mesh.sub_version = sub;
  }
  if (ver >= 4) {
    uint32_t bsp_size = 0;
    nav_file.read((char*)(&bsp_size), sizeof(bsp_size));
    mesh.save_bsp_size = bsp_size;
  }
  if (ver >= 14) {
    uint8_t analyzed = 0;
    nav_file.read((char*)(&analyzed), sizeof(analyzed));
    mesh.analyzed = analyzed;
  }

  
  // "Places callouts" - Melody
  if (ver >= 5) {
    uint16_t place_count = 0;
    nav_file.read((char*)(&place_count), sizeof(place_count));
    mesh.places.reserve(place_count);
    for (uint16_t i = 0; i < place_count; ++i) {
      uint16_t len = 0;
      nav_file.read((char*)(&len), sizeof(len));
      std::string name;
      name.resize(len);
      if (len > 0) {
        nav_file.read(name.data(), len);
      }
      mesh.places.push_back(name);
    }
    if (ver > 11) {
      uint8_t has_unnamed = 0;
      nav_file.read((char*)(&has_unnamed), sizeof(has_unnamed));
      mesh.has_unnamed_areas = (has_unnamed != 0);
    }
  }

  
  // Parse all areas
  uint32_t area_count = 0;
  nav_file.read((char*)(&area_count), sizeof(area_count));
  mesh.areas.reserve(area_count);
  for (uint32_t i = 0; i < area_count; ++i) {
    Area a{};
    nav_file.read((char*)(&a.id), sizeof(a.id));
    if (ver >= 13) {
      nav_file.read((char*)(&a.attributes), sizeof(uint32_t));
    } else if (ver >= 9) {
      uint16_t attr16 = 0; nav_file.read((char*)(&attr16), sizeof(attr16)); a.attributes = attr16;
    } else {
      uint8_t attr8 = 0; nav_file.read((char*)(&attr8), sizeof(attr8)); a.attributes = attr8;
    }

    // NW, SE
    nav_file.read((char*)(&a.nw[0]), sizeof(float)*3);
    nav_file.read((char*)(&a.se[0]), sizeof(float)*3);
    // NE, SW Z
    nav_file.read((char*)(&a.ne_z), sizeof(float));
    nav_file.read((char*)(&a.sw_z), sizeof(float));

    // NESW area connections
    for (int d = 0; d < 4; ++d) {
      uint32_t cnt = 0; nav_file.read((char*)(&cnt), sizeof(cnt));
      a.connections[d].resize(cnt);
      for (uint32_t k = 0; k < cnt; ++k) {
        nav_file.read((char*)(&a.connections[d][k]), sizeof(uint32_t));
      }
    }

    // Hiding spots
    uint8_t hide_cnt = 0; nav_file.read((char*)(&hide_cnt), sizeof(hide_cnt));
    a.hiding_spots.reserve(hide_cnt);
    for (uint8_t h = 0; h < hide_cnt; ++h) {
      HidingSpot hs{};
      nav_file.read((char*)(&hs.id), sizeof(hs.id));
      nav_file.read((char*)(&hs.pos[0]), sizeof(float)*3);
      nav_file.read((char*)(&hs.attrs), sizeof(uint8_t));
      a.hiding_spots.push_back(hs);
    }

    if (ver < 15) {
      uint8_t approach_cnt = 0; nav_file.read((char*)(&approach_cnt), sizeof(approach_cnt));
      // Each approachSpot_t is 4+4+1+4+1 bytes;
      for (uint8_t ap = 0; ap < approach_cnt; ++ap) {
        uint32_t tmpu32; uint8_t tmpu8;
        nav_file.read((char*)(&tmpu32), 4);
        nav_file.read((char*)(&tmpu32), 4);
        nav_file.read((char*)(&tmpu8), 1);
        nav_file.read((char*)(&tmpu32), 4);
        nav_file.read((char*)(&tmpu8), 1);
      }
    }

    uint32_t enc_cnt = 0; nav_file.read((char*)(&enc_cnt), sizeof(enc_cnt));
    for (uint32_t e = 0; e < enc_cnt; ++e) {
      // encounterPath_t has two uints + two bytes + spots btw
      uint32_t tmpu32; uint8_t tmpu8; uint8_t spot_cnt;
      nav_file.read((char*)(&tmpu32), 4); // Entryareaid
      nav_file.read((char*)(&tmpu8), 1);  // Entrydirection
      nav_file.read((char*)(&tmpu32), 4); // Destareaid
      nav_file.read((char*)(&tmpu8), 1);  // Destdirection
      nav_file.read((char*)(&spot_cnt), 1);
      for (uint8_t s = 0; s < spot_cnt; ++s) {
        nav_file.read((char*)(&tmpu32), 4); // Areaid
        nav_file.read((char*)(&tmpu8), 1);  // Parametricdistance
      }
    }

    // placeid
    uint16_t place = 0; nav_file.read((char*)(&place), sizeof(place));
    a.place_id = place;

    // lodderz (ladders useless bcuz there are no ladders in tf2 but navmesh system has them so why not lol)
    for (int v = 0; v < 2; ++v) {
      uint32_t lc = 0; nav_file.read((char*)(&lc), sizeof(lc));
      a.ladder_ids[v].resize(lc);
      for (uint32_t li = 0; li < lc; ++li) {
        nav_file.read((char*)(&a.ladder_ids[v][li]), sizeof(uint32_t));
      }
    }

    //earliest occupy times (2 floats), light intenstties (4 floats)
    nav_file.read((char*)(&a.earliest_occupy[0]), sizeof(float)*2);
    nav_file.read((char*)(&a.light_intensity[0]), sizeof(float)*4);

    // area binds
    uint32_t bind_cnt = 0; nav_file.read((char*)(&bind_cnt), sizeof(bind_cnt));
    a.binds.reserve(bind_cnt);
    for (uint32_t bi = 0; bi < bind_cnt; ++bi) {
      AreaBind b{};
      nav_file.read((char*)(&b.target_area_id), sizeof(uint32_t));
      nav_file.read((char*)(&b.flags), sizeof(uint8_t));
      a.binds.push_back(b);
    }

    // inherit vis from area id
    nav_file.read((char*)(&a.inherit_vis_from), sizeof(uint32_t));

    // area flags (navmesh 16)
    if (mesh.version >= 16 && mesh.sub_version == 2) {
      nav_file.read((char*)(&a.tf_attribute_flags), sizeof(uint32_t));
    }

    mesh.area_index_by_id[a.id] = mesh.areas.size();
    mesh.areas.push_back(std::move(a));
  }

  // Ladders
  if (ver >= 6) {
    uint32_t ladder_count = 0; nav_file.read((char*)(&ladder_count), sizeof(ladder_count));
    mesh.ladders.reserve(ladder_count);
    for (uint32_t li = 0; li < ladder_count; ++li) {
      Ladder l{};
      nav_file.read((char*)(&l.id), sizeof(l.id));
      nav_file.read((char*)(&l.width), sizeof(l.width));
      nav_file.read((char*)(&l.top[0]), sizeof(float)*3);
      nav_file.read((char*)(&l.bottom[0]), sizeof(float)*3);
      nav_file.read((char*)(&l.length), sizeof(l.length));
      nav_file.read((char*)(&l.direction), sizeof(l.direction));
      if (ver == 6) {
        nav_file.read((char*)(&l.dangling), sizeof(uint8_t));
      } else {
        l.dangling = 0;
      }
      nav_file.read((char*)(&l.top_forward_area), sizeof(uint32_t));
      nav_file.read((char*)(&l.top_left_area), sizeof(uint32_t));
      nav_file.read((char*)(&l.top_right_area), sizeof(uint32_t));
      nav_file.read((char*)(&l.top_behind_area), sizeof(uint32_t));
      nav_file.read((char*)(&l.bottom_area), sizeof(uint32_t));
      mesh.ladders.push_back(l);
    }
  }

  nav_file.close();
}



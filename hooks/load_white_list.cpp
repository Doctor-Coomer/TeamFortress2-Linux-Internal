#include "../gui/config.hpp"

void* (*load_white_list_original)(void*, const char*);

void* load_white_list_hook(void* me, const char* str) {
  if (config.misc.exploits.bypasspure == true) {
    return nullptr;
  }

  return load_white_list_original(me, str);
}

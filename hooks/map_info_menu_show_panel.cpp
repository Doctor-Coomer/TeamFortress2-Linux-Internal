#include <unistd.h>

#include "../gui/config.hpp"

void (*map_info_menu_show_panel_original)(void*, bool) = NULL;

void map_info_menu_show_panel_hook(void* me, bool show) {
  if (config.misc.automation.auto_class_select == true) {
    map_info_menu_show_panel_original(me, false);
  } else {
    map_info_menu_show_panel_original(me, show);
  }
}

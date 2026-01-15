//55 48 8D 15 ? ? ? ? 48 89 E5 41 54 41 89 F4 53 48 8B 07

#include <unistd.h>

#include "../gui/config.hpp"

void (*text_window_show_panel_original)(void*, bool) = NULL;

void text_window_show_panel_hook(void* me, bool show) {
  if (config.misc.automation.auto_class_select == true) {
    text_window_show_panel_original(me, false);
  } else {
    text_window_show_panel_original(me, show);
  }    
}

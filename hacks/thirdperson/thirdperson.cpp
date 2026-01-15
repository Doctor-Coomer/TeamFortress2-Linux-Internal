#include "../../gui/config.hpp"

#include "../../classes/player.hpp"
#include "../../math.hpp"

#include "../../interfaces/entity_list.hpp"
#include "../../interfaces/input.hpp"
#include "../../interfaces/engine_trace.hpp"

void trace_hull(Vec3 vecStart, Vec3 vecEnd, Vec3 vecHullMin, Vec3 vecHullMax, unsigned int mask) {
	ray_t ray = engine_trace->init_ray(&vecStart, &vecEnd, &vecHullMin, &vecHullMax);
	struct trace_filter filter;
	struct trace_t trace;

	engine_trace->trace_ray(&ray, mask, &filter, &trace);
}

// Doesn't transition smoothly between first and third.
// Using the input interface with a properly hooked ShouldDraw methods would be better.
void thirdperson(view_setup* setup) {
  Player* localplayer = entity_list->get_localplayer();
  if (localplayer == nullptr) return;

  if (config.visuals.thirdperson.enabled == false) {
    localplayer->set_thirdperson(false);
    return;
  }

  static bool was_pressed = false;
  static bool do_thirdperson = false;
  if (config.visuals.thirdperson.enabled == true) {
    if (!was_pressed && is_button_down(config.visuals.thirdperson.key)) {
      do_thirdperson = !do_thirdperson;
      was_pressed = true;
    } else if (was_pressed && !is_button_down(config.visuals.thirdperson.key)) {
      was_pressed = false;
    }
  }

  if (do_thirdperson == true) localplayer->set_thirdperson(true);
  else localplayer->set_thirdperson(false);

  if (input->is_thirdperson()) {
    Vec3 forward, right, up; angle_vectors(setup->angles, &forward, &right, &up);

    Vec3 offset;
    offset += right * config.visuals.thirdperson.x;
    offset += up * config.visuals.thirdperson.y;
    offset -= forward * config.visuals.thirdperson.z;

    Vec3 origin = localplayer->get_shoot_pos(); //pView->origin
    Vec3 start = origin;
    Vec3 end = origin + offset;

    float hull = 9.f;
    Vec3 mins = { -hull, -hull, -hull }, maxs = { hull, hull, hull };

    struct trace_t trace;
    engine_trace->trace_hull(&start, &end, &mins, &maxs, MASK_SOLID, &trace);
    end = trace.endpos;
    
    setup->origin = end;
  }

}

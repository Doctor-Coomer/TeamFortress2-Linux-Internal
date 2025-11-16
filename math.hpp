#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>

#include "vec.hpp"

//180/pi
#define radpi 57.295779513082f

//pi/180
#define pideg 0.017453293f

inline static float distance_3d(Vec3 location_one, Vec3 location_two) {
  return sqrt(((location_one.y - location_two.y)*(location_one.y - location_two.y)) +
	      ((location_one.x - location_two.x)*(location_one.x - location_two.x)) +
	      ((location_one.z - location_two.z)*(location_one.z - location_two.z)));
}

inline static float distance_squared_2d(Vec3 location_one, Vec3 location_two) {
  return (location_one.x - location_two.x)*(location_one.x - location_two.x) + (location_one.y - location_two.y)*(location_one.y - location_two.y);
}

inline static float distance_2d(Vec3 a, Vec3 b) {
  return std::sqrt(distance_squared_2d(a, b));
}

inline static float azimuth_to_signed(float yaw) {
  yaw = std::fmod(yaw, 360.0f);
  if (yaw > 180.0f) yaw -= 360.0f;
  if (yaw <= -180.0f) yaw += 360.0f;
  return yaw;
}

inline static void angle_vectors(Vec3 angles, Vec3* forward, Vec3* right, Vec3* up) {
  double sp, sy, sr, cp, cy, cr;
  sincos(angles.x * pideg, &sp, &cp);
  sincos(angles.y * pideg, &sy, &cy);

  if (forward) {
    forward->x = cp * cy;
    forward->y = cp * sy;
    forward->z = -sp;
  }

  if (right || up) {
    sincos(angles.z * pideg, &sr, &cr);

    if (right) {
      right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
      right->y = (-1 * sr * sp * sy + -1 * cr * cy);
      right->z = -1 * sr * cp;
    }

    if (up) {
      up->x = (cr * sp * cy + -sr * -sy);
      up->y = (cr * sp * sy + -sr * cy);
      up->z = cr * cp;
    }
  }
}

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

#endif

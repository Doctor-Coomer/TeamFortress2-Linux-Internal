#include <cmath> 

#ifndef VEC_HPP
#define VEC_HPP

struct Vec2 {
  int x = 0, y = 0;
};

struct Vec3 {
  float x = 0.0, y = 0.0, z = 0.0;
  
  float length() const {
    return sqrtf(x * x + y * y + z * z);
  }

  void normalize() {
    float len = length();
    if (len != 0.0f) {
      x /= len;
      y /= len;
      z /= len;
    }
  }

  float dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  Vec3 operator+(const Vec3 v) {
    return Vec3{x + v.x, y + v.y, z + v.z};
  }

  Vec3 operator*(const Vec3 v) {
    return Vec3{x * v.x, y * v.y, z * v.z};
  }

  Vec3 operator*(float v) {
    return Vec3{x * v, y * v, z * v};
  }

  Vec3 operator*(int v) {
    return Vec3{x * v, y * v, z * v};
  }

  Vec3& operator-=(const Vec3 v) {
    x -= v.x; y -= v.y; z -= v.z; return *this;
  }

  Vec3& operator+=(const Vec3 v) {
    x += v.x; y += v.y; z += v.z; return *this;
  }

  Vec3& operator+=(float v) {
    x += v; y += v; z += v; return *this;
  }

  Vec3 operator-(const Vec3& v) const { 
  return Vec3{x - v.x, y - v.y, z - v.z};
  }

  bool operator!=(const Vec3 v) {
    return (this->x != v.x && this->y != v.y && this->z != v.z);
  }
};

inline float dist_sqr(const Vec3& p1, const Vec3& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) +
           (p1.y - p2.y) * (p1.y - p2.y) +
           (p1.z - p2.z) * (p1.z - p2.z);
}

struct __attribute__((aligned(16))) Vec3_aligned {
  float x = 0.0, y = 0.0, z = 0.0;
};

struct RGBA {
  int r = 255, g = 255, b = 255, a = 255;
};

struct RGBA_float {
  float r = 1.0, g = 1.0, b = 1.0, a = 1.0;

  RGBA to_RGBA() {
    return RGBA{.r = int(r * 255), .g = int(g * 255), .b = int(b * 255), .a = int(a * 255)};
  }

  float* to_arr() {
    return (float*)this;
  }
};

typedef float VMatrix[4][4];


struct view_setup {
  // left side of view window
  int x;
  int m_nUnscaledX;
  // top side of view window
  int y;
  int m_nUnscaledY;
  // width of view window
  int width;
  int m_nUnscaledWidth;
  // height of view window
  int height;
  // which eye are we rendering?
  int m_eStereoEye;
  int m_nUnscaledHeight;

  // the rest are only used by 3D views

  // Orthographic projection?
  bool		m_bOrtho;
  // View-space rectangle for ortho projection.
  float		m_OrthoLeft;
  float		m_OrthoTop;
  float		m_OrthoRight;
  float		m_OrthoBottom;

  // horizontal FOV in degrees
  float		fov;
  // horizontal FOV in degrees for in-view model
  float		fovViewmodel;

  // 3D origin of camera
  Vec3		origin;

  // heading of camera (pitch, yaw, roll)
  Vec3		angles;
  // local Z coordinate of near plane of camera
  float		zNear;
  // local Z coordinate of far plane of camera
  float		zFar;

  // local Z coordinate of near plane of camera ( when rendering view model )
  float		zNearViewmodel;
  // local Z coordinate of far plane of camera ( when rendering view model )
  float		zFarViewmodel;

  // set to true if this is to draw into a subrect of the larger screen
  // this really is a hack, but no more than the rest of the way this class is used
  bool		m_bRenderToSubrectOfLargerScreen;

  // The aspect ratio to use for computing the perspective projection matrix
  // (0.0f means use the viewport)
  float		m_flAspectRatio;

  // Controls for off-center projection (needed for poster rendering)
  bool		m_bOffCenter;
  float		m_flOffCenterTop;
  float		m_flOffCenterBottom;
  float		m_flOffCenterLeft;
  float		m_flOffCenterRight;

  // Control that the SFM needs to tell the engine not to do certain post-processing steps
  bool		m_bDoBloomAndToneMapping;

  // Cached mode for certain full-scene per-frame varying state such as sun entity coverage
  bool		m_bCacheFullSceneState;

  // If using VR, the headset calibration will feed you a projection matrix per-eye.
  // This does NOT override the Z range - that will be set up as normal (i.e. the values in this matrix will be ignored).
  bool        m_bViewToProjectionOverride;
  VMatrix     m_ViewToProjection;
};

template<typename T>
inline T vtable_fn(void* base, int index) {
  void** vtable = *(void***)base;
  return (T)vtable[index];
}

#endif

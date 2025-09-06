#ifndef VEC_HPP
#define VEC_HPP
#include <limits>
#include <cmath>

struct Vec2 {
    int x = 0, y = 0;
};

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    Vec3 operator+(const Vec3 v) const {
        return Vec3{x + v.x, y + v.y, z + v.z};
    }

    Vec3 operator*(const Vec3 v) const {
        return Vec3{x * v.x, y * v.y, z * v.z};
    }

    Vec3 operator*(const float v) const {
        return Vec3{x * v, y * v, z * v};
    }

    Vec3 operator*(const int v) const {
        return Vec3{x * v, y * v, z * v};
    }

    Vec3 &operator-=(const Vec3 v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3 &operator+=(const Vec3 v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3 &operator+=(const float v) {
        x += v;
        y += v;
        z += v;
        return *this;
    }

    Vec3 operator-(const Vec3 v) const {
        return Vec3{x - v.x, y - v.y, z - v.z};

    }

    float length2d(void) const
    {
        return sqrtf(x * x + y * y);
    }

    float normalize2d()
    {
        const float length = length2d();

        const float length_normal = 1.f / (std::numeric_limits<float>::epsilon() + length);

        x *= length_normal;
        y *= length_normal;
        z = 0.f;

        return length;
    }
};

struct __attribute__((aligned(16))) Vec3_aligned {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct RGBA {
    int r = 255, g = 255, b = 255, a = 255;
};

struct RGBA_float {
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

    [[nodiscard]] RGBA to_RGBA() const {
        return RGBA{
            .r = static_cast<int>(r * 255), .g = static_cast<int>(g * 255), .b = static_cast<int>(b * 255),
            .a = static_cast<int>(a * 255)
        };
    }

    float *to_arr() {
        return reinterpret_cast<float *>(this);
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
    bool m_bOrtho;
    // View-space rectangle for ortho projection.
    float m_OrthoLeft;
    float m_OrthoTop;
    float m_OrthoRight;
    float m_OrthoBottom;

    // horizontal FOV in degrees
    float fov;
    // horizontal FOV in degrees for in-view model
    float fovViewmodel;

    // 3D origin of camera
    Vec3 origin;

    // heading of camera (pitch, yaw, roll)
    Vec3 angles;
    // local Z coordinate of near plane of camera
    float zNear;
    // local Z coordinate of far plane of camera
    float zFar;

    // local Z coordinate of near plane of camera ( when rendering view model )
    float zNearViewmodel;
    // local Z coordinate of far plane of camera ( when rendering view model )
    float zFarViewmodel;

    // set to true if this is to draw into a subrect of the larger screen
    // this really is a hack, but no more than the rest of the way this class is used
    bool m_bRenderToSubrectOfLargerScreen;

    // The aspect ratio to use for computing the perspective projection matrix
    // (0.0f means use the viewport)
    float m_flAspectRatio;

    // Controls for off-center projection (needed for poster rendering)
    bool m_bOffCenter;
    float m_flOffCenterTop;
    float m_flOffCenterBottom;
    float m_flOffCenterLeft;
    float m_flOffCenterRight;

    // Control that the SFM needs to tell the engine not to do certain post-processing steps
    bool m_bDoBloomAndToneMapping;

    // Cached mode for certain full-scene per-frame varying state such as sun entity coverage
    bool m_bCacheFullSceneState;

    // If using VR, the headset calibration will feed you a projection matrix per-eye.
    // This does NOT override the Z range - that will be set up as normal (i.e. the values in this matrix will be ignored).
    bool m_bViewToProjectionOverride;
    VMatrix m_ViewToProjection;
};

#endif

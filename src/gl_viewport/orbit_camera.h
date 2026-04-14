#pragma once
// orbit_camera.h — Orbit camera with pan/zoom for 3D viewport widgets.

namespace gl_viewport {

struct OrbitCamera {
    float yaw = 30.0f;       // degrees
    float pitch = 25.0f;     // degrees
    float distance = 20.0f;
    float panX = 0.0f;
    float panY = 0.0f;
    float centerX = 0.0f;    // orbit pivot point
    float centerY = 0.0f;
    float centerZ = 0.0f;

    void orbit(int dx, int dy);
    void pan(int dx, int dy);
    void zoom(float delta);   // positive = zoom in
    void frameBounds(const float* bmin, const float* bmax);
    void applyGLTransform() const;
    void getPosition(float* out) const;
    void getBasis(float* fwd, float* right, float* up) const;
};

} // namespace gl_viewport

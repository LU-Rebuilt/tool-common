#include "orbit_camera.h"

#include <QOpenGLFunctions>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace gl_viewport {

void OrbitCamera::orbit(int dx, int dy) {
    yaw += dx * 0.5f;
    pitch += dy * 0.5f;
    pitch = std::clamp(pitch, -89.0f, 89.0f);
}

void OrbitCamera::pan(int dx, int dy) {
    float scale = distance * 0.002f;
    panX += dx * scale;
    panY -= dy * scale;
}

void OrbitCamera::zoom(float delta) {
    distance -= delta * distance * 0.1f;
    distance = std::max(0.5f, distance);
}

void OrbitCamera::frameBounds(const float* bmin, const float* bmax) {
    if (bmin[0] > bmax[0]) return;
    centerX = (bmin[0] + bmax[0]) * 0.5f;
    centerY = (bmin[1] + bmax[1]) * 0.5f;
    centerZ = (bmin[2] + bmax[2]) * 0.5f;
    float dx = bmax[0] - bmin[0], dy = bmax[1] - bmin[1], dz = bmax[2] - bmin[2];
    distance = std::sqrt(dx*dx + dy*dy + dz*dz) * 0.5f * 2.5f;
    if (distance < 1.0f) distance = 1.0f;
    panX = panY = 0.0f;
}

void OrbitCamera::applyGLTransform() const {
    glTranslatef(panX, panY, -distance);
    glRotatef(pitch, 1, 0, 0);
    glRotatef(yaw, 0, 1, 0);
    glTranslatef(-centerX, -centerY, -centerZ);
}

void OrbitCamera::getPosition(float* out) const {
    float yr = yaw * static_cast<float>(M_PI) / 180.0f;
    float pr = pitch * static_cast<float>(M_PI) / 180.0f;
    float sy = std::sin(yr), cy = std::cos(yr);
    float sp = std::sin(pr), cp = std::cos(pr);
    out[0] = centerX + distance * cp * sy - panX * cy;
    out[1] = centerY + distance * sp + panY;
    out[2] = centerZ + distance * cp * cy + panX * sy;
}

void OrbitCamera::getBasis(float* fwd, float* right, float* up) const {
    float yr = yaw * static_cast<float>(M_PI) / 180.0f;
    float pr = pitch * static_cast<float>(M_PI) / 180.0f;
    float sy = std::sin(yr), cy = std::cos(yr);
    float sp = std::sin(pr), cp = std::cos(pr);
    fwd[0] = -cp * sy; fwd[1] = -sp; fwd[2] = -cp * cy;
    right[0] = cy; right[1] = 0; right[2] = -sy;
    up[0] = right[1]*fwd[2] - right[2]*fwd[1];
    up[1] = right[2]*fwd[0] - right[0]*fwd[2];
    up[2] = right[0]*fwd[1] - right[1]*fwd[0];
}

} // namespace gl_viewport

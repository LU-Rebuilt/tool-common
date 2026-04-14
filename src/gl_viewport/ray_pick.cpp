#include "ray_pick.h"

#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace gl_viewport {

bool rayTriangleIntersect(const float* orig, const float* dir,
                           const float* v0, const float* v1, const float* v2,
                           float& t) {
    float e1[3] = {v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]};
    float e2[3] = {v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]};
    float h[3] = {dir[1]*e2[2]-dir[2]*e2[1], dir[2]*e2[0]-dir[0]*e2[2], dir[0]*e2[1]-dir[1]*e2[0]};
    float a = e1[0]*h[0]+e1[1]*h[1]+e1[2]*h[2];
    if (std::fabs(a) < 1e-8f) return false;
    float f = 1.0f/a;
    float s[3] = {orig[0]-v0[0], orig[1]-v0[1], orig[2]-v0[2]};
    float u = f*(s[0]*h[0]+s[1]*h[1]+s[2]*h[2]);
    if (u < 0 || u > 1) return false;
    float q[3] = {s[1]*e1[2]-s[2]*e1[1], s[2]*e1[0]-s[0]*e1[2], s[0]*e1[1]-s[1]*e1[0]};
    float v = f*(dir[0]*q[0]+dir[1]*q[1]+dir[2]*q[2]);
    if (v < 0 || u+v > 1) return false;
    t = f*(e2[0]*q[0]+e2[1]*q[1]+e2[2]*q[2]);
    return t > 0;
}

void unprojectMouseRay(int mouseX, int mouseY, int viewW, int viewH,
                        const OrbitCamera& camera,
                        float* rayOrigin, float* rayDir) {
    float aspect = (viewH > 0) ? static_cast<float>(viewW) / viewH : 1.0f;
    float tanHalf = std::tan(45.0f * 0.5f * static_cast<float>(M_PI) / 180.0f);
    float nx = (2.0f * mouseX / viewW - 1.0f) * aspect * tanHalf;
    float ny = (1.0f - 2.0f * mouseY / viewH) * tanHalf;

    camera.getPosition(rayOrigin);

    float fwd[3], right[3], up[3];
    camera.getBasis(fwd, right, up);

    rayDir[0] = fwd[0] + nx*right[0] + ny*up[0];
    rayDir[1] = fwd[1] + nx*right[1] + ny*up[1];
    rayDir[2] = fwd[2] + nx*right[2] + ny*up[2];
    float len = std::sqrt(rayDir[0]*rayDir[0]+rayDir[1]*rayDir[1]+rayDir[2]*rayDir[2]);
    rayDir[0]/=len; rayDir[1]/=len; rayDir[2]/=len;
}

int pickMesh(int mouseX, int mouseY, int viewW, int viewH,
              const OrbitCamera& camera,
              const std::vector<RenderMesh>& meshes) {
    float orig[3], dir[3];
    unprojectMouseRay(mouseX, mouseY, viewW, viewH, camera, orig, dir);

    float bestT = std::numeric_limits<float>::max();
    int bestIdx = -1;
    for (int mi = 0; mi < static_cast<int>(meshes.size()); ++mi) {
        const auto& mesh = meshes[mi];
        if (!mesh.visible) continue;
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
            const float* v0 = &mesh.vertices[mesh.indices[i]*3];
            const float* v1 = &mesh.vertices[mesh.indices[i+1]*3];
            const float* v2 = &mesh.vertices[mesh.indices[i+2]*3];
            float t;
            if (rayTriangleIntersect(orig, dir, v0, v1, v2, t) && t < bestT) {
                bestT = t;
                bestIdx = mi;
            }
        }
    }
    return bestIdx;
}

} // namespace gl_viewport

#pragma once
// ray_pick.h — Ray-triangle intersection and mouse unprojection for picking.

#include "orbit_camera.h"
#include "render_mesh.h"

#include <vector>

namespace gl_viewport {

// Moller-Trumbore ray-triangle intersection. Returns true if hit, sets t.
bool rayTriangleIntersect(const float* orig, const float* dir,
                           const float* v0, const float* v1, const float* v2,
                           float& t);

// Unproject a mouse position to a world-space ray.
void unprojectMouseRay(int mouseX, int mouseY, int viewW, int viewH,
                        const OrbitCamera& camera,
                        float* rayOrigin, float* rayDir);

// Pick the nearest mesh hit by a mouse click. Returns mesh index or -1.
int pickMesh(int mouseX, int mouseY, int viewW, int viewH,
              const OrbitCamera& camera,
              const std::vector<RenderMesh>& meshes);

} // namespace gl_viewport

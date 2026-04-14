#pragma once
// render_mesh.h — Shared pre-tessellated triangle mesh for GL rendering.

#include <cstdint>
#include <string>
#include <vector>

namespace gl_viewport {

struct RenderMesh {
    std::vector<float> vertices;     // xyz interleaved, world space
    std::vector<float> normals;      // xyz interleaved (optional — empty = unlit)
    std::vector<uint32_t> indices;   // triangle indices
    float color[4] = {1, 1, 1, 0.35f};
    float wireColor[3] = {1, 1, 1};
    bool visible = true;
    std::string label;
};

} // namespace gl_viewport

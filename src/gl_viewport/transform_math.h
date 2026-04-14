#pragma once
// transform_math.h — 3x3+translation transform math for GL viewport rendering.

#include <array>

namespace gl_viewport {

// Compact 3x3 rotation + translation, stored as column vectors.
// Layout matches Havok convention so tools can cast directly from HKX data.
struct Transform {
    struct Vec3 { float x = 0, y = 0, z = 0; };
    struct Vec4 { float x = 0, y = 0, z = 0, w = 1; };

    Vec3 col0{1, 0, 0};
    Vec3 col1{0, 1, 0};
    Vec3 col2{0, 0, 1};
    Vec4 translation{0, 0, 0, 1};
};

// TransformPoint: result = col0*x + col1*y + col2*z + translation
std::array<float, 3> transformPoint(const Transform& t,
                                     float x, float y, float z);

// CombineTransforms: result = parent * child
// Rotation: R_result = R_parent * R_child
// Translation: T_result = R_parent * T_child + T_parent
Transform combineTransforms(const Transform& parent,
                             const Transform& child);

} // namespace gl_viewport

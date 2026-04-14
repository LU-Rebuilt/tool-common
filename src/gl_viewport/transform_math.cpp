#include "transform_math.h"

namespace gl_viewport {

std::array<float, 3> transformPoint(const Transform& t,
                                     float x, float y, float z) {
    return {
        t.col0.x * x + t.col1.x * y + t.col2.x * z + t.translation.x,
        t.col0.y * x + t.col1.y * y + t.col2.y * z + t.translation.y,
        t.col0.z * x + t.col1.z * y + t.col2.z * z + t.translation.z
    };
}

Transform combineTransforms(const Transform& parent,
                             const Transform& child) {
    Transform result;
    for (int row = 0; row < 3; row++) {
        float pRow[3];
        if (row == 0) { pRow[0] = parent.col0.x; pRow[1] = parent.col1.x; pRow[2] = parent.col2.x; }
        else if (row == 1) { pRow[0] = parent.col0.y; pRow[1] = parent.col1.y; pRow[2] = parent.col2.y; }
        else { pRow[0] = parent.col0.z; pRow[1] = parent.col1.z; pRow[2] = parent.col2.z; }
        float c0 = pRow[0]*child.col0.x + pRow[1]*child.col0.y + pRow[2]*child.col0.z;
        float c1 = pRow[0]*child.col1.x + pRow[1]*child.col1.y + pRow[2]*child.col1.z;
        float c2 = pRow[0]*child.col2.x + pRow[1]*child.col2.y + pRow[2]*child.col2.z;
        if (row == 0) { result.col0.x = c0; result.col1.x = c1; result.col2.x = c2; }
        else if (row == 1) { result.col0.y = c0; result.col1.y = c1; result.col2.y = c2; }
        else { result.col0.z = c0; result.col1.z = c1; result.col2.z = c2; }
    }
    auto childT = transformPoint(parent, child.translation.x,
                                  child.translation.y, child.translation.z);
    result.translation = {childT[0], childT[1], childT[2], 1.0f};
    return result;
}

} // namespace gl_viewport

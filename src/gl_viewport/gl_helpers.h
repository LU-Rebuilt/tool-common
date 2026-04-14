#pragma once
// gl_helpers.h — Shared GL utility functions for legacy fixed-function rendering.

namespace gl_viewport {

// Set up a perspective projection matrix (replaces gluPerspective).
void glSetPerspective(float fovY, float aspect, float zNear, float zFar);

// Draw a ground grid at Y=0.
void drawGrid(float extent, float spacing);

// Draw RGB axes at origin.
void drawAxes(float length);

} // namespace gl_viewport

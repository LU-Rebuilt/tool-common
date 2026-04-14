#include "gl_helpers.h"

#include <QOpenGLFunctions>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace gl_viewport {

void glSetPerspective(float fovY, float aspect, float zNear, float zFar) {
    float top = zNear * std::tan(fovY * 0.5f * static_cast<float>(M_PI) / 180.0f);
    float right = top * aspect;
    glFrustum(static_cast<double>(-right), static_cast<double>(right),
              static_cast<double>(-top), static_cast<double>(top),
              static_cast<double>(zNear), static_cast<double>(zFar));
}

void drawGrid(float extent, float spacing) {
    glBegin(GL_LINES);
    glColor4f(0.3f, 0.3f, 0.3f, 0.3f);
    for (float i = -extent; i <= extent; i += spacing) {
        glVertex3f(i, 0, -extent);
        glVertex3f(i, 0, extent);
        glVertex3f(-extent, 0, i);
        glVertex3f(extent, 0, i);
    }
    glEnd();
}

void drawAxes(float length) {
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor3f(1, 0.2f, 0.2f); glVertex3f(0,0,0); glVertex3f(length,0,0);
    glColor3f(0.2f, 1, 0.2f); glVertex3f(0,0,0); glVertex3f(0,length,0);
    glColor3f(0.2f, 0.2f, 1); glVertex3f(0,0,0); glVertex3f(0,0,length);
    glEnd();
    glLineWidth(1);
}

} // namespace gl_viewport

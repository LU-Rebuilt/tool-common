#include "gl_viewport_widget.h"
#include "gl_helpers.h"
#include "ray_pick.h"

#include <QOpenGLFunctions>

#include <algorithm>
#include <cmath>

namespace gl_viewport {

BaseGLViewport::BaseGLViewport(QWidget* parent) : QOpenGLWidget(parent) {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
}

// ---------------------------------------------------------------------------
// Mesh management
// ---------------------------------------------------------------------------

void BaseGLViewport::addMesh(RenderMesh mesh) {
    meshes_.push_back(std::move(mesh));
    update();
}

void BaseGLViewport::clearMeshes() {
    meshes_.clear();
    selectedIdx_ = -1;
    update();
}

void BaseGLViewport::removeMesh(int idx) {
    if (idx >= 0 && idx < static_cast<int>(meshes_.size())) {
        meshes_.erase(meshes_.begin() + idx);
        if (selectedIdx_ == idx) selectedIdx_ = -1;
        else if (selectedIdx_ > idx) selectedIdx_--;
        update();
    }
}

void BaseGLViewport::setSelectedIndex(int idx) {
    if (idx != selectedIdx_) {
        selectedIdx_ = idx;
        update();
    }
}

void BaseGLViewport::setMeshVisible(int idx, bool visible) {
    if (idx >= 0 && idx < static_cast<int>(meshes_.size())) {
        meshes_[idx].visible = visible;
        update();
    }
}

bool BaseGLViewport::isMeshVisible(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(meshes_.size()))
        return meshes_[idx].visible;
    return false;
}

// ---------------------------------------------------------------------------
// Camera / bounds
// ---------------------------------------------------------------------------

void BaseGLViewport::meshBounds(const RenderMesh& mesh, float* bmin, float* bmax) {
    bmin[0] = bmin[1] = bmin[2] = 1e30f;
    bmax[0] = bmax[1] = bmax[2] = -1e30f;
    for (size_t i = 0; i + 2 < mesh.vertices.size(); i += 3) {
        for (int k = 0; k < 3; k++) {
            bmin[k] = std::min(bmin[k], mesh.vertices[i + k]);
            bmax[k] = std::max(bmax[k], mesh.vertices[i + k]);
        }
    }
}

void BaseGLViewport::computeVisibleBounds(float* bmin, float* bmax) const {
    bmin[0] = bmin[1] = bmin[2] = 1e30f;
    bmax[0] = bmax[1] = bmax[2] = -1e30f;
    for (const auto& m : meshes_) {
        if (!m.visible || m.vertices.empty()) continue;
        float mb[3], mB[3];
        meshBounds(m, mb, mB);
        for (int k = 0; k < 3; k++) {
            bmin[k] = std::min(bmin[k], mb[k]);
            bmax[k] = std::max(bmax[k], mB[k]);
        }
    }
}

void BaseGLViewport::fitToVisible() {
    float bmin[3], bmax[3];
    computeVisibleBounds(bmin, bmax);
    camera_.frameBounds(bmin, bmax);
    update();
}

void BaseGLViewport::fitToMesh(int idx) {
    if (idx < 0 || idx >= static_cast<int>(meshes_.size())) return;
    float mb[3], mB[3];
    meshBounds(meshes_[idx], mb, mB);
    camera_.frameBounds(mb, mB);
    update();
}

// ---------------------------------------------------------------------------
// OpenGL
// ---------------------------------------------------------------------------

void BaseGLViewport::initializeGL() {
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    onInitGL();
}

void BaseGLViewport::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (h > 0) ? static_cast<float>(w) / h : 1.0f;
    glSetPerspective(45.0f, aspect, 1.0f, 100000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void BaseGLViewport::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera_.applyGLTransform();

    drawBackground();

    // Two-pass rendering for each visible mesh
    for (int mi = 0; mi < static_cast<int>(meshes_.size()); ++mi) {
        const auto& mesh = meshes_[mi];
        if (mesh.vertices.empty() || mesh.indices.empty()) continue;
        if (!mesh.visible) continue;
        if (!shouldDrawMesh(mi)) continue;

        bool selected = (mi == selectedIdx_);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh.vertices.data());

        // Pass 1: solid fill
        if (showSolid) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, 1.0f);
            if (selected)
                glColor4f(1.0f, 0.4f, 0.1f, 0.6f);
            else
                glColor4f(mesh.color[0], mesh.color[1], mesh.color[2], mesh.color[3]);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()),
                           GL_UNSIGNED_INT, mesh.indices.data());
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // Pass 2: wireframe overlay
        if (showWireframe) {
            if (selected)
                glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            else
                glColor4f(mesh.wireColor[0], mesh.wireColor[1], mesh.wireColor[2], 1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(selected ? 2.0f : 1.0f);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()),
                           GL_UNSIGNED_INT, mesh.indices.data());
            glLineWidth(1.0f);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    drawOverlay();
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void BaseGLViewport::mousePressEvent(QMouseEvent* event) {
    lastMouse_ = event->pos();
    pressPos_ = event->pos();
}

void BaseGLViewport::mouseMoveEvent(QMouseEvent* event) {
    int dx = event->pos().x() - lastMouse_.x();
    int dy = event->pos().y() - lastMouse_.y();
    lastMouse_ = event->pos();

    if (event->buttons() & Qt::LeftButton) {
        camera_.orbit(dx, dy);
    } else if (event->buttons() & Qt::RightButton) {
        camera_.pan(dx, dy);
    } else if (event->buttons() & Qt::MiddleButton) {
        camera_.distance += dy * 0.5f;
        camera_.distance = std::max(0.5f, camera_.distance);
    }
    update();
}

void BaseGLViewport::mouseReleaseEvent(QMouseEvent* event) {
    int dx = event->pos().x() - pressPos_.x();
    int dy = event->pos().y() - pressPos_.y();
    bool isClick = (dx * dx + dy * dy < 16);

    if (event->button() == Qt::LeftButton && isClick) {
        int idx = pickMesh(event->pos().x(), event->pos().y(),
                           width(), height(), camera_, meshes_);
        selectedIdx_ = idx;
        emit meshClicked(idx);
        update();
    } else if (event->button() == Qt::RightButton && isClick) {
        int idx = pickMesh(event->pos().x(), event->pos().y(),
                           width(), height(), camera_, meshes_);
        emit contextRequested(idx, event->globalPosition().toPoint());
    }
}

void BaseGLViewport::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        selectedIdx_ = -1;
        emit meshClicked(-1);
        update();
    }
    QOpenGLWidget::keyPressEvent(event);
}

void BaseGLViewport::wheelEvent(QWheelEvent* event) {
    float delta = static_cast<float>(event->angleDelta().y()) / 120.0f;
    camera_.zoom(delta);
    update();
}

} // namespace gl_viewport

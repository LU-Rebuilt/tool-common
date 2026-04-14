#pragma once
// gl_viewport_widget.h — Base QOpenGLWidget with orbit camera, two-pass
// rendering, mesh selection/visibility, and ray picking.
//
// Subclass and override the virtual hooks for tool-specific behavior.

#include "render_mesh.h"
#include "orbit_camera.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <vector>

namespace gl_viewport {

class BaseGLViewport : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit BaseGLViewport(QWidget* parent = nullptr);

    // Mesh management
    void addMesh(RenderMesh mesh);
    void clearMeshes();
    int meshCount() const { return static_cast<int>(meshes_.size()); }
    const RenderMesh& meshAt(int idx) const { return meshes_[idx]; }
    void removeMesh(int idx);

    // Selection
    int selectedIndex() const { return selectedIdx_; }
    void setSelectedIndex(int idx);

    // Visibility
    void setMeshVisible(int idx, bool visible);
    bool isMeshVisible(int idx) const;

    // Camera
    OrbitCamera& camera() { return camera_; }
    const OrbitCamera& camera() const { return camera_; }
    void fitToVisible();
    void fitToMesh(int idx);

    // Rendering options
    bool showSolid = true;
    bool showWireframe = true;

    // Bounding box helpers
    static void meshBounds(const RenderMesh& mesh, float* bmin, float* bmax);
    void computeVisibleBounds(float* bmin, float* bmax) const;

signals:
    void meshClicked(int index);     // left-click pick result
    void contextRequested(int index, QPoint globalPos); // right-click

protected:
    // QOpenGLWidget overrides with default implementations
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    // ---- Subclass hooks ----

    // Called after default GL init (depth, blend, clear color).
    virtual void onInitGL() {}

    // Draw before meshes (grid, axes, etc.).
    virtual void drawBackground() {}

    // Draw after meshes (navmesh overlay, etc.).
    virtual void drawOverlay() {}

    // Tool-specific mesh filtering. Return false to skip rendering this mesh.
    virtual bool shouldDrawMesh(int /*idx*/) const { return true; }

    std::vector<RenderMesh> meshes_;
    OrbitCamera camera_;
    int selectedIdx_ = -1;

    QPoint lastMouse_;
    QPoint pressPos_;
};

} // namespace gl_viewport

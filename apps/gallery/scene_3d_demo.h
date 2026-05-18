#pragma once

#include <core/scene/scene.h>
#include <renderer3d/renderer3d.h>
#include <renderer3d/camera.h>
#include <renderer3d/mesh.h>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::Render3D;

class Scene3DDemo : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "3DDemo"; }

private:
    Render3D::Renderer3D m_renderer3d;
    Render3D::Camera m_camera;
    Render3D::Mesh m_mesh;

    bool m_dragging = false;
    float m_lastMouseX = 0, m_lastMouseY = 0;
    float m_lightAngle = 0.0f;
    bool m_autoRotateLight = true;
    glm::vec3 m_lightPos = {50, 80, 50};
};

} // namespace CarHMI::Gallery

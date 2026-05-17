#pragma once

#include "scene/scene.h"
#include "renderer3d/renderer3d.h"
#include "renderer3d/camera.h"
#include "renderer3d/mesh.h"

namespace CarHMI {

class Scene3DDemo : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "3DDemo"; }

private:
    Renderer3D m_renderer3d;
    Camera m_camera;
    Mesh m_mesh;

    bool m_dragging = false;
    float m_lastMouseX = 0, m_lastMouseY = 0;
    float m_lightAngle = 0.0f;
    bool m_autoRotateLight = true;
    glm::vec3 m_lightPos = {50, 80, 50};
};

} // namespace CarHMI

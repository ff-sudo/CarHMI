#include "scene_3d_demo.h"
#include "application.h"
#include <core/event_bus.h>
#include <core/event.h>
#include <core/scene/scene_manager.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <cmath>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::GUI;
using namespace CarHMI::RHI;
using namespace CarHMI::Render3D;

void Scene3DDemo::OnEnter() {
    m_renderer3d.Init();

    if (!m_mesh.LoadOBJ("resources/3d/cottage_obj.obj")) {
        spdlog::error("Failed to load cottage model");
    }

    // Position camera based on model bounds
    m_camera.SetTarget(m_mesh.GetCenter());
    m_camera.SetDistance(m_mesh.GetBoundingRadius() * 2.5f);

    // Subscribe to mouse events for camera control
    EventBus::Get().subscribe<MouseButtonEvent>([this](const MouseButtonEvent& e) {
        if (e.button == MouseButton::Left) {
            m_dragging = e.pressed;
            m_lastMouseX = e.x;
            m_lastMouseY = e.y;
        }
    });

    EventBus::Get().subscribe<MouseMoveEvent>([this](const MouseMoveEvent& e) {
        if (m_dragging) {
            float dx = e.x - m_lastMouseX;
            float dy = e.y - m_lastMouseY;
            m_camera.Orbit(dx * 0.3f, -dy * 0.3f);
            m_lastMouseX = e.x;
            m_lastMouseY = e.y;
        }
    });

    EventBus::Get().subscribe<MouseScrollEvent>([this](const MouseScrollEvent& e) {
        m_camera.Zoom(e.yOffset);
    });
}

void Scene3DDemo::OnExit() {
    m_renderer3d.Shutdown();
}

void Scene3DDemo::OnUpdate(float dt) {
    if (m_autoRotateLight) {
        m_lightAngle += dt * 30.0f;
        float r = m_mesh.GetBoundingRadius() * 1.5f;
        m_lightPos = m_mesh.GetCenter() + glm::vec3(
            cosf(glm::radians(m_lightAngle)) * r,
            r,
            sinf(glm::radians(m_lightAngle)) * r
        );
    }
    m_renderer3d.SetLightPos(m_lightPos);
}

void Scene3DDemo::OnDraw() {
    auto winSize = Application::Get().GetPlatform().GetWindowSize();
    float aspect = (float)winSize.x / (float)winSize.y;

    glClear(GL_DEPTH_BUFFER_BIT);

    m_renderer3d.Begin(m_camera, aspect);
    m_renderer3d.DrawMesh(m_mesh);
    m_renderer3d.End();
}

void Scene3DDemo::OnImGui() {
    ImGui::Begin("3D Scene");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Camera: dist=%.1f yaw=%.1f pitch=%.1f",
                m_camera.GetDistance(), m_camera.GetYaw(), m_camera.GetPitch());
    ImGui::Text("Model: %d materials, radius=%.1f",
                (int)m_mesh.GetMaterials().size(), m_mesh.GetBoundingRadius());
    ImGui::Separator();
    ImGui::Checkbox("Auto Rotate Light", &m_autoRotateLight);
    ImGui::DragFloat3("Light Pos", &m_lightPos[0], 1.0f);
    ImGui::Separator();
    if (ImGui::Button("Back to Dashboard")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::End();
}

} // namespace CarHMI::Gallery

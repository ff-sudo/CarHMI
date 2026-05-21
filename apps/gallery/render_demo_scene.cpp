#include "render_demo_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

namespace CarHMI::Gallery {

void RenderDemoScene::OnEnter() {}
void RenderDemoScene::OnExit() {}
void RenderDemoScene::OnUpdate(float) {}

void RenderDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    auto& renderer = app.GetRenderer();

    renderer.SetViewportSize(winSize.x, winSize.y);
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    renderer.Begin(proj);

    // --- Rounded Rectangles Demo ---
    // Different radii
    renderer.DrawRoundedRect({50, 50}, {200, 80}, 0.0f, {0.2f, 0.6f, 0.9f, 1.0f});
    renderer.DrawRoundedRect({50, 150}, {200, 80}, 10.0f, {0.9f, 0.3f, 0.3f, 1.0f});
    renderer.DrawRoundedRect({50, 250}, {200, 80}, 25.0f, {0.3f, 0.9f, 0.4f, 1.0f});
    renderer.DrawRoundedRect({50, 350}, {200, 80}, 40.0f, {0.9f, 0.7f, 0.1f, 1.0f});

    // Pill shape (radius = height/2)
    renderer.DrawRoundedRect({50, 460}, {200, 60}, 30.0f, {0.7f, 0.3f, 0.9f, 1.0f});

    // Square with large radius (becomes circle-ish)
    renderer.DrawRoundedRect({50, 550}, {80, 80}, 40.0f, {0.3f, 0.8f, 0.8f, 1.0f});

    // --- Scissor Clipping Demo ---
    // Draw a background rect
    renderer.DrawRoundedRect({350, 50}, {300, 200}, 15.0f, {0.15f, 0.15f, 0.2f, 1.0f});

    // Clip content inside
    renderer.PushScissor({360, 60}, {280, 180});

    // Draw things that should be clipped
    renderer.DrawQuad({340, 40}, {150, 150}, {0.9f, 0.2f, 0.2f, 0.8f});
    renderer.DrawRoundedRect({400, 100}, {300, 100}, 20.0f, {0.2f, 0.8f, 0.3f, 1.0f});
    renderer.DrawCircle({600, 150}, 80.0f, {0.2f, 0.5f, 0.9f, 0.8f}, 32);

    renderer.PopScissor();

    // Nested scissor demo
    renderer.DrawRoundedRect({350, 300}, {300, 250}, 15.0f, {0.15f, 0.15f, 0.2f, 1.0f});
    renderer.PushScissor({360, 310}, {280, 230});

    renderer.DrawQuad({350, 300}, {300, 250}, {0.3f, 0.3f, 0.5f, 1.0f});

    // Nested clip (smaller area)
    renderer.PushScissor({400, 350}, {180, 130});
    renderer.DrawCircle({490, 415}, 100.0f, {0.9f, 0.6f, 0.1f, 1.0f}, 32);
    renderer.PopScissor();

    renderer.PopScissor();

    renderer.End();
}

void RenderDemoScene::OnImGui() {
    ImGui::Begin("Render Demo");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Rounded Rect: left column");
    ImGui::Text("Scissor Clip: right column");
    ImGui::Separator();
    if (ImGui::Button("< Back")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::End();
}

} // namespace CarHMI::Gallery

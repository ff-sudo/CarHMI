#include "scroll_demo_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <gui/widgets/panel.h>
#include <gui/widgets/label.h>
#include <gui/widgets/button.h>
#include <gui/debug/debug_panel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

namespace CarHMI::Gallery {

using namespace CarHMI::GUI;

void ScrollDemoScene::OnEnter() {
    m_scrollView = new ScrollView(600, {50, 50}, {350, 400});

    // Add many children to overflow both vertically and horizontally
    float y = 10;
    for (int i = 0; i < 20; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Item %d", i + 1);

        if (i % 3 == 0) {
            float hue = (float)i / 20.0f;
            // Wide panel to also create horizontal overflow
            auto* panel = new Panel(601 + i, {10, y}, {450, 50},
                {0.2f + hue * 0.3f, 0.3f, 0.6f - hue * 0.3f, 1.0f});
            m_scrollView->AddChild(panel);
        } else {
            auto* label = new Label(601 + i, {10, y}, buf);
            label->SetSize({310, 30});
            m_scrollView->AddChild(label);
        }

        y += (i % 3 == 0) ? 60 : 40;
    }
}

void ScrollDemoScene::OnExit() {
    delete m_scrollView;
    m_scrollView = nullptr;
}

void ScrollDemoScene::OnUpdate(float dt) {
    if (m_scrollView) {
        auto& ctx = Application::Get().GetUIContext();
        m_scrollView->Update(ctx);
    }
}

void ScrollDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    auto& renderer = app.GetRenderer();

    renderer.SetViewportSize(winSize.x, winSize.y);
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    renderer.Begin(proj);

    auto& ctx = app.GetUIContext();
    if (m_scrollView) m_scrollView->Draw(ctx);

    renderer.End();
}

void ScrollDemoScene::OnImGui() {
    ImGui::Begin("ScrollView Demo");

    auto& stats = Application::Get().GetRenderer().GetStats();
    Debug::ShowRenderStats(stats);
    ImGui::Separator();

    if (ImGui::Button("< Back")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::Separator();

    if (m_scrollView) {
        float offsetY = m_scrollView->GetScrollOffsetY();
        float offsetX = m_scrollView->GetScrollOffsetX();
        auto contentSize = m_scrollView->GetContentSize();
        auto viewSize = m_scrollView->GetSize();

        ImGui::Text("Content: %.0f x %.0f", contentSize.x, contentSize.y);
        ImGui::Text("View: %.0f x %.0f", viewSize.x, viewSize.y);
        ImGui::Text("Max Scroll Y: %.0f", std::max(0.0f, contentSize.y - viewSize.y));
        ImGui::Text("Max Scroll X: %.0f", std::max(0.0f, contentSize.x - viewSize.x));

        if (ImGui::SliderFloat("Scroll Y", &offsetY, 0, std::max(0.0f, contentSize.y - viewSize.y))) {
            m_scrollView->SetScrollOffsetY(offsetY);
        }
        if (ImGui::SliderFloat("Scroll X", &offsetX, 0, std::max(0.0f, contentSize.x - viewSize.x))) {
            m_scrollView->SetScrollOffsetX(offsetX);
        }

        ImGui::Text("Children: %d", (int)m_scrollView->GetChildren().size());
        ImGui::Separator();
        ImGui::Text("Mouse wheel: vertical scroll");
        ImGui::Text("Shift+wheel or H-wheel: horizontal scroll");
        ImGui::Text("Drag scrollbar thumb to scroll");
    }

    ImGui::End();
}

} // namespace CarHMI::Gallery

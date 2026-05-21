#include "layout_demo_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <gui/widgets/label.h>
#include <gui/widgets/panel.h>
#include <gui/widgets/button.h>
#include <gui/debug/debug_panel.h>
#include <glm/gtc/matrix_transform.hpp>

namespace CarHMI::Gallery {

using namespace CarHMI::GUI;

void LayoutDemoScene::OnEnter() {
    // Main container — a vertical BoxLayout
    m_root = new BoxLayout(500, {30, 30}, {500, 500}, BoxDirection::Vertical, 20.0f, 10.0f);
    m_root->SetDrawBackground(true, {0.12f, 0.12f, 0.16f, 1.0f});

    // Child 1: Fixed size panel
    m_child1 = new Panel(501, {0, 0}, {200, 60}, {0.2f, 0.5f, 0.8f, 1.0f});

    // Child 2: Percent width (80%) panel
    m_child2 = new Panel(502, {0, 0}, {100, 50}, {0.8f, 0.3f, 0.3f, 1.0f});
    m_child2->SetWidthPercent(80.0f);

    // Child 3: Fill width panel with margin
    m_child3 = new Panel(503, {0, 0}, {100, 50}, {0.3f, 0.8f, 0.4f, 1.0f});
    m_child3->SetFillWidth();
    m_child3->SetMargin(Insets(5, 10, 5, 10));

    // Child 4: Fill width + fill height (takes remaining vertical space)
    m_child4 = new Panel(504, {0, 0}, {100, 100}, {0.9f, 0.7f, 0.2f, 1.0f});
    m_child4->SetFillWidth();
    m_child4->SetFillHeight();

    m_root->AddChild(m_child1);
    m_root->AddChild(m_child2);
    m_root->AddChild(m_child3);
    m_root->AddChild(m_child4);
}

void LayoutDemoScene::OnExit() {
    delete m_root;
    m_root = nullptr;
}

void LayoutDemoScene::OnUpdate(float dt) {
    auto& ctx = Application::Get().GetUIContext();
    if (m_root) m_root->Update(ctx);
}

void LayoutDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    auto& renderer = app.GetRenderer();

    renderer.SetViewportSize(winSize.x, winSize.y);
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    renderer.Begin(proj);

    auto& ctx = app.GetUIContext();
    if (m_root) m_root->Draw(ctx);

    renderer.End();
}

void LayoutDemoScene::OnImGui() {
    ImGui::Begin("Layout Inspector");

    // Render stats
    auto& stats = Application::Get().GetRenderer().GetStats();
    Debug::ShowRenderStats(stats);
    ImGui::Separator();

    // Back button
    if (ImGui::Button("< Back")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::Separator();

    // Layout container editor
    bool layoutChanged = false;

    if (m_root) {
        layoutChanged |= Debug::EditBoxLayout("Container", m_root);

        // Container size
        auto rootSz = m_root->GetSize();
        if (ImGui::DragFloat2("Container Size", &rootSz.x, 1.0f, 100.0f, 1000.0f)) {
            m_root->SetSize(rootSz);
            layoutChanged = true;
        }

        ImGui::Separator();
        ImGui::Text("Children:");

        // Per-child editors
        layoutChanged |= Debug::EditWidget("Panel 1 (Fixed)", m_child1);
        layoutChanged |= Debug::EditWidget("Panel 2 (80%%)", m_child2);
        layoutChanged |= Debug::EditWidget("Panel 3 (Fill+Margin)", m_child3);
        layoutChanged |= Debug::EditWidget("Panel 4 (Fill Both)", m_child4);

        if (layoutChanged)
            m_root->Recalculate();

        ImGui::Separator();
        ImGui::Text("Widget Tree:");
        Debug::ShowWidgetTree(m_root);
    }

    ImGui::End();
}

} // namespace CarHMI::Gallery

#include "textinput_demo_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <gui/focus_manager.h>
#include <gui/debug/debug_panel.h>
#include <glm/gtc/matrix_transform.hpp>

namespace CarHMI::Gallery {

using namespace CarHMI::GUI;

void TextInputDemoScene::OnEnter() {
    m_input1 = new TextInput(700, {50, 80}, {300, 36}, "Enter name...");
    m_input2 = new TextInput(701, {50, 140}, {300, 36}, "Enter email...");
    m_input3 = new TextInput(702, {50, 200}, {300, 36}, "Search...");

    m_input1->SetOnSubmit([this](const std::string& text) {
        m_lastSubmit = "Name: " + text;
    });
    m_input2->SetOnSubmit([this](const std::string& text) {
        m_lastSubmit = "Email: " + text;
    });
    m_input3->SetOnSubmit([this](const std::string& text) {
        m_lastSubmit = "Search: " + text;
    });

    FocusManager::Get().ClearAll();
    m_input1->RegisterFocusRecursive();
    m_input2->RegisterFocusRecursive();
    m_input3->RegisterFocusRecursive();
}

void TextInputDemoScene::OnExit() {
    FocusManager::Get().ClearAll();
    delete m_input1; m_input1 = nullptr;
    delete m_input2; m_input2 = nullptr;
    delete m_input3; m_input3 = nullptr;
}

void TextInputDemoScene::OnUpdate(float dt) {
    auto& ctx = Application::Get().GetUIContext();
    if (m_input1) m_input1->Update(ctx);
    if (m_input2) m_input2->Update(ctx);
    if (m_input3) m_input3->Update(ctx);
}

void TextInputDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    auto& renderer = app.GetRenderer();

    renderer.SetViewportSize(winSize.x, winSize.y);
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    renderer.Begin(proj);

    auto& ctx = app.GetUIContext();

    // Labels
    auto& font = ctx.GetFont();
    font.DrawText(renderer, "Name:", 50, 55, {0.8f, 0.8f, 0.85f, 1.0f});
    font.DrawText(renderer, "Email:", 50, 115, {0.8f, 0.8f, 0.85f, 1.0f});
    font.DrawText(renderer, "Search:", 50, 175, {0.8f, 0.8f, 0.85f, 1.0f});

    if (m_input1) m_input1->Draw(ctx);
    if (m_input2) m_input2->Draw(ctx);
    if (m_input3) m_input3->Draw(ctx);

    renderer.End();
}

void TextInputDemoScene::OnImGui() {
    ImGui::Begin("TextInput Demo");

    auto& stats = Application::Get().GetRenderer().GetStats();
    Debug::ShowRenderStats(stats);
    ImGui::Separator();

    if (ImGui::Button("< Back")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::Separator();

    if (m_input1) ImGui::Text("Input 1: \"%s\"", m_input1->GetText().c_str());
    if (m_input2) ImGui::Text("Input 2: \"%s\"", m_input2->GetText().c_str());
    if (m_input3) ImGui::Text("Input 3: \"%s\"", m_input3->GetText().c_str());

    ImGui::Text("Last Submit: %s", m_lastSubmit.c_str());
    ImGui::Separator();
    ImGui::Text("Tab: switch focus between inputs");
    ImGui::Text("Arrow keys: move cursor");
    ImGui::Text("Enter: submit");
    ImGui::Text("Click: position cursor");

    ImGui::End();
}

} // namespace CarHMI::Gallery

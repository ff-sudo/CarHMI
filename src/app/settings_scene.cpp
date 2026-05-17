#include "settings_scene.h"
#include "core/application.h"
#include "scene/scene_manager.h"
#include "gui/focus_manager.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

namespace CarHMI {

void SettingsScene::OnEnter() {
    m_root = new BoxLayout(100, {50, 50}, {400, 400}, BoxDirection::Vertical, 20, 15);
    m_root->SetDrawBackground(true, {0.12f, 0.14f, 0.20f, 0.95f});

    auto* title = new Label(101, {0, 0}, "Settings", Label::Role::Title);
    title->SetSize({360, 30});

    auto* brightnessLabel = new Label(102, {0, 0}, "Brightness: 80%");
    brightnessLabel->SetSize({360, 25});

    m_brightnessSlider = new Slider(103, {0, 0}, {360, 30}, 0.0f, 100.0f);
    m_brightnessSlider->SetValue(80.0f);
    m_brightnessSlider->SetOnChanged([brightnessLabel](float val) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Brightness: %.0f%%", val);
        brightnessLabel->SetText(buf);
    });

    auto* volumeLabel = new Label(104, {0, 0}, "Volume: 50%");
    volumeLabel->SetSize({360, 25});

    auto* volumeSlider = new Slider(105, {0, 0}, {360, 30}, 0.0f, 100.0f);
    volumeSlider->SetValue(50.0f);
    volumeSlider->SetOnChanged([volumeLabel](float val) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Volume: %.0f%%", val);
        volumeLabel->SetText(buf);
    });

    auto* backBtn = new Button(106, {0, 0}, {360, 45}, "< Back");
    backBtn->SetOnClick([]() {
        spdlog::info("Back button clicked");
        Application::Get().GetSceneManager().Pop();
    });

    m_root->AddChild(title);
    m_root->AddChild(brightnessLabel);
    m_root->AddChild(m_brightnessSlider);
    m_root->AddChild(volumeLabel);
    m_root->AddChild(volumeSlider);
    m_root->AddChild(backBtn);
    m_root->SubscribeThemeChange();
    FocusManager::Get().ClearAll();
    m_root->RegisterFocusRecursive();
}

void SettingsScene::OnExit() {
    delete m_root;
    m_root = nullptr;
}

void SettingsScene::OnUpdate(float dt) {
    auto& ctx = Application::Get().GetUIContext();
    if (m_root) m_root->Update(ctx);
}

void SettingsScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);

    app.GetRenderer().Begin(proj);
    auto& ctx = app.GetUIContext();
    if (m_root) m_root->Draw(ctx);
    app.GetRenderer().End();
}

void SettingsScene::OnImGui() {
    ImGui::Begin("Settings Debug");
    ImGui::Text("Brightness: %.0f", m_brightnessSlider ? m_brightnessSlider->GetValue() : 0);
    ImGui::End();
}

} // namespace CarHMI

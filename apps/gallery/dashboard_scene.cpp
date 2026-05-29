#include "dashboard_scene.h"
#include "settings_scene.h"
#include "widget_showcase_scene.h"
#include "scene_3d_demo.h"
#include "render_demo_scene.h"
#include "layout_demo_scene.h"
#include "timer_demo_scene.h"
#include "canvas_demo_scene.h"
#include "scroll_demo_scene.h"
#include "textinput_demo_scene.h"
#include "image_view_demo_scene.h"
#include "dialog_demo_scene.h"
#include "application.h"
#include <core/property/binding.h>
#include <core/scene/scene_manager.h>
#include <gui/focus_manager.h>
#include <core/animation/animation_manager.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <memory>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::GUI;
using namespace CarHMI::RHI;

void DashboardScene::OnEnter() {
    m_root = new BoxLayout(1, {50, 50}, {400, 580}, BoxDirection::Vertical, 20, 12);
    m_root->SetDrawBackgroundFromTheme(true);

    auto& i = I18n::Get();

    auto* title = new Label(2, {0, 0}, i.T("app.title"), Label::Role::Title);
    title->SetSize({360, 30});
    title->SetI18nKey("app.title");

    m_speedLabel = new Label(3, {0, 0}, i.T("speed.zero"));
    m_speedLabel->SetSize({360, 25});
    m_speedLabel->SetI18nKey("speed.zero");

    m_speedSlider = new Slider(4, {0, 0}, {360, 30}, 0.0f, 240.0f);
    m_speedBar = new ProgressBar(5, {0, 0}, {360, 25});

    auto* fuelLabel = new Label(6, {0, 0}, i.T("fuel.default"));
    fuelLabel->SetSize({360, 25});
    fuelLabel->SetI18nKey("fuel.default");

    auto* fuelSlider = new Slider(11, {0, 0}, {360, 30}, 0.0f, 100.0f);
    fuelSlider->SetValue(75.0f);

    m_fuelBar = new ProgressBar(7, {0, 0}, {360, 25});
    m_fuelBar->SetValue(0.75f);
    m_fuelBar->SetColors({0.9f, 0.7f, 0.1f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});

    auto* btnGrid = new BoxLayout(20, {0, 0}, {360, 180}, BoxDirection::Vertical, 0, 4);

    auto* btnRow1 = new HBoxLayout(21, {0, 0}, {360, 40}, 0, 4);
    auto* btnRow2 = new HBoxLayout(22, {0, 0}, {360, 40}, 0, 4);
    auto* btnRow3 = new HBoxLayout(23, {0, 0}, {360, 40}, 0, 4);
    auto* btnRow4 = new HBoxLayout(24, {0, 0}, {360, 40}, 0, 4);

    auto* settingsBtn = new Button(8, {0, 0}, {50, 40}, i.T("btn.settings"));
    settingsBtn->SetI18nKey("btn.settings");
    settingsBtn->SetFillWidth();
    settingsBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<SettingsScene>());
    });

    auto* widgetsBtn = new Button(12, {0, 0}, {50, 40}, i.T("btn.widgets"));
    widgetsBtn->SetI18nKey("btn.widgets");
    widgetsBtn->SetFillWidth();
    widgetsBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<WidgetShowcaseScene>());
    });

    auto* btn3D = new Button(13, {0, 0}, {50, 40}, "3D");
    btn3D->SetFillWidth();
    btn3D->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<Scene3DDemo>());
    });

    auto* renderBtn = new Button(14, {0, 0}, {50, 40}, "Render");
    renderBtn->SetFillWidth();
    renderBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<RenderDemoScene>());
    });

    auto* layoutBtn = new Button(15, {0, 0}, {50, 40}, "Layout");
    layoutBtn->SetFillWidth();
    layoutBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<LayoutDemoScene>());
    });

    auto* resetBtn = new Button(9, {0, 0}, {50, 40}, i.T("btn.reset"));
    resetBtn->SetI18nKey("btn.reset");
    resetBtn->SetFillWidth();

    auto* timerBtn = new Button(16, {0, 0}, {50, 40}, "Timer");
    timerBtn->SetFillWidth();
    timerBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<TimerDemoScene>());
    });

    auto* canvasBtn = new Button(17, {0, 0}, {50, 40}, "Canvas");
    canvasBtn->SetFillWidth();
    canvasBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<CanvasDemoScene>());
    });

    auto* scrollBtn = new Button(18, {0, 0}, {50, 40}, "Scroll");
    scrollBtn->SetFillWidth();
    scrollBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<ScrollDemoScene>());
    });

    auto* inputBtn = new Button(19, {0, 0}, {50, 40}, "Input");
    inputBtn->SetFillWidth();
    inputBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<TextInputDemoScene>());
    });

    auto* imageBtn = new Button(25, {0, 0}, {50, 40}, "Image");
    imageBtn->SetFillWidth();
    imageBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<ImageViewDemoScene>());
    });

    auto* dialogBtn = new Button(26, {0, 0}, {50, 40}, "Dialog");
    dialogBtn->SetFillWidth();
    dialogBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Push(std::make_unique<DialogDemoScene>());
    });

    btnRow1->AddChild(settingsBtn);
    btnRow1->AddChild(widgetsBtn);
    btnRow1->AddChild(btn3D);
    btnRow2->AddChild(renderBtn);
    btnRow2->AddChild(layoutBtn);
    btnRow2->AddChild(timerBtn);
    btnRow3->AddChild(canvasBtn);
    btnRow3->AddChild(scrollBtn);
    btnRow3->AddChild(inputBtn);
    btnRow4->AddChild(imageBtn);
    btnRow4->AddChild(dialogBtn);
    btnRow4->AddChild(resetBtn);
    btnGrid->AddChild(btnRow1);
    btnGrid->AddChild(btnRow2);
    btnGrid->AddChild(btnRow3);
    btnGrid->AddChild(btnRow4);

    m_statusLabel = new Label(10, {0, 0}, i.T("status.ready"), Label::Role::Subtitle);
    m_statusLabel->SetSize({360, 25});
    m_statusLabel->SetI18nKey("status.ready");

    // Data bindings: slider value -> label text + progress bar
    m_bindingScope.UnbindAll();

    m_bindingScope.BindFormat(m_speedSlider->ValueProperty(), m_speedLabel->TextProperty(), "Speed: %.0f km/h");
    m_bindingScope.Bind<float, float>(m_speedSlider->ValueProperty(), m_speedBar->ValueProperty(),
        [](const float& val) { return val / 240.0f; });
    m_bindingScope.BindFormat(fuelSlider->ValueProperty(), fuelLabel->TextProperty(), "Fuel: %.0f%%");
    m_bindingScope.Bind<float, float>(fuelSlider->ValueProperty(), m_fuelBar->ValueProperty(),
        [](const float& val) { return val / 100.0f; });

    // Color thresholds still use callbacks
    m_speedSlider->ValueProperty().OnChanged([this](const float&, const float& val) {
        if (val > 180)
            m_speedBar->SetColors({0.9f, 0.2f, 0.2f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});
        else if (val > 120)
            m_speedBar->SetColors({0.9f, 0.7f, 0.1f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});
        else
            m_speedBar->SetColors({0.2f, 0.8f, 0.4f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});
    });

    fuelSlider->ValueProperty().OnChanged([this](const float&, const float& val) {
        if (val < 15)
            m_fuelBar->SetColors({0.9f, 0.2f, 0.2f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});
        else if (val < 30)
            m_fuelBar->SetColors({0.9f, 0.7f, 0.1f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});
        else
            m_fuelBar->SetColors({0.2f, 0.8f, 0.4f, 1.0f}, {0.2f, 0.2f, 0.25f, 1.0f});
    });

    resetBtn->SetOnClick([this, fuelSlider]() {
        m_speedSlider->SetValue(0);
        fuelSlider->SetValue(75.0f);
        m_statusLabel->SetText(I18n::Get().T("status.reset"));
    });

    m_root->AddChild(title);
    m_root->AddChild(m_speedLabel);
    m_root->AddChild(m_speedSlider);
    m_root->AddChild(m_speedBar);
    m_root->AddChild(fuelLabel);
    m_root->AddChild(fuelSlider);
    m_root->AddChild(m_fuelBar);
    m_root->AddChild(btnGrid);
    m_root->AddChild(m_statusLabel);
    m_root->SubscribeThemeChange();
    FocusManager::Get().ClearAll();
    m_root->RegisterFocusRecursive();
}

void DashboardScene::OnExit() {
    delete m_root;
    m_root = nullptr;
}

void DashboardScene::OnUpdate(float dt) {
    auto& ctx = Application::Get().GetUIContext();
    if (m_root) m_root->Update(ctx);
}

void DashboardScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);

    app.GetRenderer().Begin(proj);
    auto& ctx = app.GetUIContext();
    if (m_root) m_root->Draw(ctx);
    app.GetRenderer().End();
}

void DashboardScene::OnImGui() {
    ImGui::Begin("Debug Panel");
    auto& stats = Application::Get().GetRenderer().GetStats();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
    ImGui::Separator();
    ImGui::Text("Speed: %.1f", m_speedSlider ? m_speedSlider->GetValue() : 0);
    ImGui::Text("Scene Stack: %d", Application::Get().GetSceneManager().StackSize());
    ImGui::Text("Active Animations: %d", Application::Get().GetAnimationManager().ActiveCount());
    if (Application::Get().GetSceneManager().IsTransitioning())
        ImGui::Text("Transitioning: offset=%.1f", Application::Get().GetSceneManager().GetTransitionOffset());
    ImGui::Separator();
    ImGui::Text("Theme: %s", ThemeManager::Get().GetActiveThemeName().c_str());
    if (ImGui::Button("Dark")) ThemeManager::Get().SetActiveTheme("dark");
    ImGui::SameLine();
    if (ImGui::Button("Light")) ThemeManager::Get().SetActiveTheme("light");
    ImGui::Text("(Edit JSON for hot-reload)");
    ImGui::Separator();
    ImGui::Text("Focused Widget: %d", FocusManager::Get().GetFocusedId());
    ImGui::Text("Tab/Arrows=navigate, Enter=activate, Scroll=adjust");
    ImGui::Separator();
    ImGui::Text("Language: %s", I18n::Get().GetLanguageCode().c_str());
    ImGui::Text("Direction: %s", I18n::Get().IsRTL() ? "RTL" : "LTR");
    if (ImGui::Button("EN")) I18n::Get().SetLanguage("en");
    ImGui::SameLine();
    if (ImGui::Button("ZH")) I18n::Get().SetLanguage("zh");
    ImGui::SameLine();
    if (ImGui::Button("AR")) I18n::Get().SetLanguage("ar");
    ImGui::Separator();
    ImGui::Text("Font:");
    if (ImGui::Button("Arial")) Application::Get().ReloadFont("resources/fonts/arial.ttf", 24.0f);
    ImGui::SameLine();
    if (ImGui::Button("Consolas")) Application::Get().ReloadFont("resources/fonts/consola.ttf", 24.0f);
    ImGui::SameLine();
    if (ImGui::Button("MSYH")) Application::Get().ReloadFont("resources/fonts/msyh.ttc", 24.0f);
    ImGui::SameLine();
    if (ImGui::Button("CHS Boot")) Application::Get().ReloadFont("resources/fonts/chs_boot_EX.ttf", 24.0f);
    ImGui::End();
}

} // namespace CarHMI::Gallery

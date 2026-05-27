#include "widget_showcase_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <gui/focus_manager.h>
#include <gui/i18n/i18n.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <memory>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::GUI;
using namespace CarHMI::RHI;

void WidgetShowcaseScene::OnEnter() {
    // Left column: Gauge + Knob
    auto* leftCol = new BoxLayout(200, {30, 50}, {300, 620}, BoxDirection::Vertical, 15, 20);
    leftCol->SetDrawBackgroundFromTheme(true);

    auto* gaugeTitle = new Label(201, {0, 0}, "Speedometer", Label::Role::Title);
    gaugeTitle->SetI18nKey("showcase.speedometer");
    gaugeTitle->SetSize({270, 25});

    m_gauge = new Gauge(202, {0, 0}, 120.0f, 0.0f, 240.0f);
    m_gauge->SetUnit("km/h");

    auto* knobTitle = new Label(203, {0, 0}, "Volume", Label::Role::Subtitle);
    knobTitle->SetI18nKey("showcase.volume");
    knobTitle->SetSize({270, 25});

    m_knob = new Knob(204, {0, 0}, 50.0f, 0.0f, 100.0f);
    m_knob->SetValue(50.0f);

    auto* knobLabel = new Label(205, {0, 0}, "50%");
    knobLabel->SetSize({270, 20});

    m_knob->SetOnChanged([knobLabel](float val) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f%%", val);
        knobLabel->SetText(buf);
    });

    leftCol->AddChild(gaugeTitle);
    leftCol->AddChild(m_gauge);
    leftCol->AddChild(knobTitle);
    leftCol->AddChild(m_knob);
    leftCol->AddChild(knobLabel);

    // Right column: TabBar + Toggle + List
    auto* rightCol = new BoxLayout(300, {360, 50}, {350, 620}, BoxDirection::Vertical, 15, 12);
    rightCol->SetDrawBackgroundFromTheme(true);

    auto* tabBar = new TabBar(301, {0, 0}, {320, 40});
    tabBar->AddTabI18n("tab.media");
    tabBar->AddTabI18n("tab.phone");
    tabBar->AddTabI18n("tab.nav");

    auto* tabLabel = new Label(302, {0, 0}, "Tab: Media", Label::Role::Subtitle);
    tabLabel->SetSize({320, 25});
    tabBar->SetOnTabChanged([tabLabel, tabBar](int idx) {
        const char* names[] = {"Media", "Phone", "Nav"};
        char buf[32];
        snprintf(buf, sizeof(buf), "Tab: %s", names[idx]);
        tabLabel->SetText(buf);
    });

    auto* toggleRow = new HBoxLayout(310, {0, 0}, {320, 40}, 0, 15);
    auto* acLabel = new Label(311, {0, 0}, "A/C");
    acLabel->SetI18nKey("toggle.ac");
    acLabel->SetSize({30, 30});
    auto* acToggle = new Toggle(312, {0, 0}, {55, 28});

    auto* lightLabel = new Label(313, {0, 0}, "Lights");
    lightLabel->SetI18nKey("toggle.lights");
    lightLabel->SetSize({45, 30});
    auto* lightToggle = new Toggle(314, {0, 0}, {55, 28});
    lightToggle->SetOn(true);

    toggleRow->AddChild(acLabel);
    toggleRow->AddChild(acToggle);
    toggleRow->AddChild(lightLabel);
    toggleRow->AddChild(lightToggle);

    auto* listTitle = new Label(320, {0, 0}, "Stations", Label::Role::Subtitle);
    listTitle->SetI18nKey("showcase.stations");
    listTitle->SetSize({320, 25});

    auto* list = new ListWidget(321, {0, 0}, {320, 250}, 36.0f);
    list->AddItem("FM 88.1 - Classic Rock");
    list->AddItem("FM 91.5 - Jazz");
    list->AddItem("FM 95.3 - Pop Hits");
    list->AddItem("FM 98.7 - News Radio");
    list->AddItem("FM 101.1 - Country");
    list->AddItem("FM 104.3 - Electronic");
    list->AddItem("FM 107.9 - Classical");
    list->AddItem("AM 720 - Sports Talk");
    list->AddItem("AM 1010 - Weather");

    auto* backBtn = new Button(399, {0, 0}, {320, 40}, "< Back to Dashboard");
    backBtn->SetI18nKey("btn.back");
    backBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Pop();
    });

    rightCol->AddChild(tabBar);
    rightCol->AddChild(tabLabel);
    rightCol->AddChild(toggleRow);
    rightCol->AddChild(listTitle);
    rightCol->AddChild(list);
    rightCol->AddChild(backBtn);

    // Root container (holds both columns, no layout - manual positioning)
    m_root = new BoxLayout(100, {0, 0}, {740, 700}, BoxDirection::Horizontal, 0, 0);
    m_root->AddChild(leftCol);
    m_root->AddChild(rightCol);
    m_root->SubscribeThemeChange();
    FocusManager::Get().ClearAll();
    m_root->RegisterFocusRecursive();
}

void WidgetShowcaseScene::OnExit() {
    delete m_root;
    m_root = nullptr;
}

void WidgetShowcaseScene::OnUpdate(float dt) {
    auto& ctx = Application::Get().GetUIContext();
    if (m_root) m_root->Update(ctx);
}

void WidgetShowcaseScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);

    app.GetRenderer().Begin(proj);
    auto& ctx = app.GetUIContext();
    if (m_root) m_root->Draw(ctx);
    app.GetRenderer().End();
}

void WidgetShowcaseScene::OnImGui() {
    ImGui::Begin("Widget Showcase");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Gauge: %.1f", m_gauge ? m_gauge->GetValue() : 0);
    ImGui::Text("Knob: %.1f", m_knob ? m_knob->GetValue() : 0);
    ImGui::Separator();
    if (m_gauge) {
        float v = m_gauge->GetValue();
        if (ImGui::SliderFloat("Speed", &v, 0, 240))
            m_gauge->SetValue(v);
    }
    ImGui::End();
}

} // namespace CarHMI::Gallery

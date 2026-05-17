#pragma once

#include "scene/scene.h"
#include "gui/layout/box_layout.h"
#include "gui/widgets/label.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/button.h"

namespace CarHMI {

class SettingsScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "Settings"; }

private:
    BoxLayout* m_root = nullptr;
    Slider* m_brightnessSlider = nullptr;
};

} // namespace CarHMI

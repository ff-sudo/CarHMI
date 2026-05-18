#pragma once

#include <core/scene/scene.h>
#include <gui/layout/box_layout.h>
#include <gui/widgets/label.h>
#include <gui/widgets/slider.h>
#include <gui/widgets/button.h>
#include <gui/widgets/progress_bar.h>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::GUI;
using namespace CarHMI::RHI;

class DashboardScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "Dashboard"; }

private:
    BoxLayout* m_root = nullptr;
    Label* m_speedLabel = nullptr;
    Slider* m_speedSlider = nullptr;
    ProgressBar* m_speedBar = nullptr;
    ProgressBar* m_fuelBar = nullptr;
    Label* m_statusLabel = nullptr;
};

} // namespace CarHMI::Gallery

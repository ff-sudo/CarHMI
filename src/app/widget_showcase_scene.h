#pragma once

#include "scene/scene.h"
#include "gui/layout/box_layout.h"
#include "gui/widgets/gauge.h"
#include "gui/widgets/knob.h"
#include "gui/widgets/toggle.h"
#include "gui/widgets/tab_bar.h"
#include "gui/widgets/list_widget.h"
#include "gui/widgets/label.h"
#include "gui/widgets/button.h"

namespace CarHMI {

class WidgetShowcaseScene : public Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "WidgetShowcase"; }

private:
    BoxLayout* m_root = nullptr;
    Gauge* m_gauge = nullptr;
    Knob* m_knob = nullptr;
};

} // namespace CarHMI

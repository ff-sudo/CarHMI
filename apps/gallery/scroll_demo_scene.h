#pragma once

#include <core/scene/scene.h>
#include <gui/widgets/scroll_view.h>

namespace CarHMI::Gallery {

class ScrollDemoScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "ScrollDemo"; }

private:
    GUI::ScrollView* m_scrollView = nullptr;
};

} // namespace CarHMI::Gallery

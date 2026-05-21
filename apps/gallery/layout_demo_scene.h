#pragma once

#include <core/scene/scene.h>
#include <gui/layout/box_layout.h>

namespace CarHMI::Gallery {

class LayoutDemoScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "LayoutDemo"; }

private:
    GUI::BoxLayout* m_root = nullptr;

    // Direct references for ImGui editing
    GUI::Widget* m_child1 = nullptr;
    GUI::Widget* m_child2 = nullptr;
    GUI::Widget* m_child3 = nullptr;
    GUI::Widget* m_child4 = nullptr;
};

} // namespace CarHMI::Gallery

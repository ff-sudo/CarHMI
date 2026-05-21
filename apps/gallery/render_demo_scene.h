#pragma once

#include <core/scene/scene.h>

namespace CarHMI::Gallery {

class RenderDemoScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
};

} // namespace CarHMI::Gallery

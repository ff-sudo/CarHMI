#pragma once

#include <core/scene/scene.h>

namespace CarHMI::Gallery {

class CanvasDemoScene : public Core::Scene {
public:
    void OnEnter() override {}
    void OnExit() override {}
    void OnUpdate(float dt) override {}
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "CanvasDemo"; }

private:
    // ImGui tweakable params
    float m_strokeWidth = 2.0f;
    float m_roundRadius = 15.0f;
    float m_arcStart = 0.0f;
    float m_arcEnd = 270.0f;
    int m_segments = 32;
    float m_fillColor[4] = {0.2f, 0.6f, 0.9f, 1.0f};
    float m_strokeColor[4] = {0.9f, 0.9f, 0.9f, 1.0f};
};

} // namespace CarHMI::Gallery

#pragma once
#include <core/scene/scene.h>

namespace CarHMI::Gallery {
class ImageViewDemoScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "ImageViewDemo"; }
private:
    struct Impl;
    Impl* m = nullptr;
};
}

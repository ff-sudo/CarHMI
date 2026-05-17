#pragma once

#include <string>

namespace CarHMI {

class Application;

class Scene {
public:
    virtual ~Scene() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnPause() {}
    virtual void OnResume() {}

    virtual void OnUpdate(float dt) {}
    virtual void OnDraw() {}
    virtual void OnImGui() {}

    virtual const char* GetName() const { return "Scene"; }
};

} // namespace CarHMI

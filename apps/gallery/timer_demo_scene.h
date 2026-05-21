#pragma once

#include <core/scene/scene.h>
#include <core/connection.h>

namespace CarHMI::Gallery {

class TimerDemoScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override {}
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "TimerDemo"; }

private:
    Core::ConnectionGroup m_timerConnections;

    // Demo state
    int m_timeoutCount = 0;
    int m_intervalCount = 0;
    bool m_timeoutActive = false;
    bool m_intervalActive = false;
    int m_timeoutDelayMs = 2000;
    int m_intervalMs = 500;
    char m_log[512] = "";
};

} // namespace CarHMI::Gallery

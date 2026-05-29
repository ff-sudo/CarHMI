#include "timer_demo_scene.h"
#include "application.h"
#include <core/timer/timer_manager.h>
#include <core/scene/scene_manager.h>
#include <gui/debug/debug_panel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

namespace CarHMI::Gallery {

void TimerDemoScene::OnEnter() {
    snprintf(m_log, sizeof(m_log), "Timer Demo ready.");
}

void TimerDemoScene::OnExit() {
    // Connection RAII: all timers auto-cancelled
    m_timerConnections.DisconnectAll();
}

void TimerDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    auto& renderer = app.GetRenderer();

    renderer.SetViewportSize(winSize.x, winSize.y);
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    renderer.Begin(proj);

    // Visual feedback: colored boxes that react to timer events
    float x = 50.0f;
    float y = 50.0f;

    // Timeout indicator
    renderer.DrawRoundedRect({x, y}, {200, 80}, 10.0f,
        m_timeoutActive ? glm::vec4(0.9f, 0.6f, 0.1f, 1.0f) : glm::vec4(0.2f, 0.2f, 0.25f, 1.0f));

    // Interval indicator - flash based on count
    float intensity = (m_intervalCount % 2 == 0) ? 0.3f : 0.9f;
    renderer.DrawRoundedRect({x, y + 100}, {200, 80}, 10.0f,
        m_intervalActive ? glm::vec4(0.2f, intensity, 0.4f, 1.0f) : glm::vec4(0.2f, 0.2f, 0.25f, 1.0f));

    renderer.End();
}

void TimerDemoScene::OnImGui() {
    ImGui::Begin("Timer Demo");

    auto& stats = Application::Get().GetRenderer().GetStats();
    GUI::Debug::ShowRenderStats(stats);
    ImGui::Separator();

    if (ImGui::Button("< Back")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::Separator();

    // --- Timeout ---
    ImGui::Text("=== SetTimeout ===");
    ImGui::SliderInt("Delay (ms)", &m_timeoutDelayMs, 100, 5000);
    ImGui::Text("Fired count: %d", m_timeoutCount);
    ImGui::Text("Status: %s", m_timeoutActive ? "WAITING..." : "idle");

    if (ImGui::Button("Start Timeout")) {
        m_timeoutActive = true;
        snprintf(m_log, sizeof(m_log), "Timeout started: %d ms", m_timeoutDelayMs);
        m_timerConnections.Add(
            Application::Get().GetTimerManager().SetTimeout([this]() {
                m_timeoutCount++;
                m_timeoutActive = false;
                snprintf(m_log, sizeof(m_log), "Timeout fired! (total: %d)", m_timeoutCount);
            }, m_timeoutDelayMs)
        );
    }

    ImGui::Separator();

    // --- Interval ---
    ImGui::Text("=== SetInterval ===");
    ImGui::SliderInt("Interval (ms)", &m_intervalMs, 100, 3000);
    ImGui::Text("Tick count: %d", m_intervalCount);
    ImGui::Text("Status: %s", m_intervalActive ? "RUNNING" : "stopped");

    if (!m_intervalActive) {
        if (ImGui::Button("Start Interval")) {
            m_intervalActive = true;
            m_intervalCount = 0;
            snprintf(m_log, sizeof(m_log), "Interval started: %d ms", m_intervalMs);
            m_timerConnections.Add(
                Application::Get().GetTimerManager().SetInterval([this]() {
                    m_intervalCount++;
                    snprintf(m_log, sizeof(m_log), "Interval tick #%d", m_intervalCount);
                }, m_intervalMs)
            );
        }
    } else {
        if (ImGui::Button("Stop Interval")) {
            m_timerConnections.DisconnectAll();
            m_intervalActive = false;
            snprintf(m_log, sizeof(m_log), "Interval stopped at tick #%d", m_intervalCount);
        }
    }

    ImGui::Separator();

    // --- Info ---
    ImGui::Text("Active timers: %d", Application::Get().GetTimerManager().ActiveCount());
    ImGui::Text("Log: %s", m_log);

    if (ImGui::Button("Clear All Timers")) {
        m_timerConnections.DisconnectAll();
        m_timeoutActive = false;
        m_intervalActive = false;
        snprintf(m_log, sizeof(m_log), "All timers cleared.");
    }

    ImGui::End();
}

} // namespace CarHMI::Gallery

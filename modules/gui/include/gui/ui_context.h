#pragma once

#include <glm/glm.hpp>
#include <core/connection.h>
#include <rhi/batch_renderer2d.h>
#include <rhi/font.h>

namespace CarHMI::Core {
class AnimationManager;
class TimerManager;
}

namespace CarHMI::GUI {

class UIContext {
public:
    void Init(RHI::BatchRenderer2D* renderer, RHI::Font* font);

    RHI::BatchRenderer2D& GetRenderer() { return *m_renderer; }
    RHI::Font& GetFont() { return *m_font; }

    // Scissor clipping
    void PushClipRect(glm::vec2 pos, glm::vec2 size);
    void PopClipRect();

    glm::vec2 GetMousePos() const { return m_mousePos; }
    bool IsMouseDown() const { return m_mouseDown; }
    bool IsMousePressed() const { return m_mousePressed; }
    bool IsMouseReleased() const { return m_mouseReleased; }
    float GetDeltaTime() const { return m_deltaTime; }

    void SetActive(int id) { m_activeId = id; }
    void SetHot(int id) { m_hotId = id; }
    int GetActive() const { return m_activeId; }
    int GetHot() const { return m_hotId; }
    void ClearActive() { m_activeId = 0; }

    void BeginFrame();

    // Service locators (non-owning, wired by Application)
    void SetAnimationManager(Core::AnimationManager* mgr) { m_animMgr = mgr; }
    Core::AnimationManager& GetAnimationManager() { return *m_animMgr; }
    bool HasAnimationManager() const { return m_animMgr != nullptr; }

    void SetTimerManager(Core::TimerManager* mgr) { m_timerMgr = mgr; }
    Core::TimerManager& GetTimerManager() { return *m_timerMgr; }
    bool HasTimerManager() const { return m_timerMgr != nullptr; }

private:
    RHI::BatchRenderer2D* m_renderer = nullptr;
    RHI::Font* m_font = nullptr;
    Core::AnimationManager* m_animMgr = nullptr;
    Core::TimerManager* m_timerMgr = nullptr;

    glm::vec2 m_mousePos = {0, 0};
    bool m_mouseDown = false;
    bool m_mousePressed = false;
    bool m_mouseReleased = false;

    float m_deltaTime = 1.0f / 60.0f;

    int m_hotId = 0;
    int m_activeId = 0;

    Core::ConnectionGroup m_connections;
};

} // namespace CarHMI::GUI

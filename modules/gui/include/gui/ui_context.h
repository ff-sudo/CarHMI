#pragma once

#include <glm/glm.hpp>
#include <rhi/batch_renderer2d.h>
#include <rhi/font.h>

namespace CarHMI::GUI {

class UIContext {
public:
    void Init(RHI::BatchRenderer2D* renderer, RHI::Font* font);

    RHI::BatchRenderer2D& GetRenderer() { return *m_renderer; }
    RHI::Font& GetFont() { return *m_font; }

    glm::vec2 GetMousePos() const { return m_mousePos; }
    bool IsMouseDown() const { return m_mouseDown; }
    bool IsMousePressed() const { return m_mousePressed; }
    bool IsMouseReleased() const { return m_mouseReleased; }

    void SetActive(int id) { m_activeId = id; }
    void SetHot(int id) { m_hotId = id; }
    int GetActive() const { return m_activeId; }
    int GetHot() const { return m_hotId; }
    void ClearActive() { m_activeId = 0; }

    void BeginFrame();

private:
    RHI::BatchRenderer2D* m_renderer = nullptr;
    RHI::Font* m_font = nullptr;

    glm::vec2 m_mousePos = {0, 0};
    bool m_mouseDown = false;
    bool m_mousePressed = false;
    bool m_mouseReleased = false;

    int m_hotId = 0;
    int m_activeId = 0;
};

} // namespace CarHMI::GUI

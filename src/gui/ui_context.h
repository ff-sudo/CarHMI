#pragma once

#include <glm/glm.hpp>
#include "renderer/batch_renderer2d.h"
#include "renderer/font.h"

namespace CarHMI {

class UIContext {
public:
    void Init(BatchRenderer2D* renderer, Font* font);

    BatchRenderer2D& GetRenderer() { return *m_renderer; }
    Font& GetFont() { return *m_font; }

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
    BatchRenderer2D* m_renderer = nullptr;
    Font* m_font = nullptr;

    glm::vec2 m_mousePos = {0, 0};
    bool m_mouseDown = false;
    bool m_mousePressed = false;
    bool m_mouseReleased = false;

    int m_hotId = 0;
    int m_activeId = 0;
};

} // namespace CarHMI

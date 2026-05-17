#pragma once

#include "../widget.h"
#include "gui/style/theme_manager.h"

namespace CarHMI {

class Panel : public Widget {
public:
    Panel(int id, glm::vec2 pos, glm::vec2 size, glm::vec4 bgColor = {-1, -1, -1, -1})
        : Widget(id, pos, size) {
        if (bgColor.r < 0)
            m_bgColor = ThemeManager::Get().GetTheme().panel.backgroundColor;
        else
            m_bgColor = bgColor;
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        ctx.GetRenderer().DrawQuad(abs, m_size, m_bgColor);
        Widget::Draw(ctx);
    }

    void SetBgColor(const glm::vec4& c) { m_bgColor = c; }

private:
    glm::vec4 m_bgColor;
};

} // namespace CarHMI

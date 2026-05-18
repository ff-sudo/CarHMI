#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>

namespace CarHMI::GUI {

enum class BoxDirection { Vertical, Horizontal };

class BoxLayout : public Widget {
public:
    BoxLayout(int id, glm::vec2 pos, glm::vec2 size,
              BoxDirection dir = BoxDirection::Vertical,
              float padding = 10.0f, float spacing = 8.0f)
        : Widget(id, pos, size), m_direction(dir), m_padding(padding), m_spacing(spacing) {}

    void AddChild(Widget* child) {
        Widget::AddChild(child);
        Recalculate();
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        if (m_drawBackground) {
            glm::vec2 abs = GetAbsolutePos();
            ctx.GetRenderer().DrawQuad(abs, m_size, m_bgColor);
        }
        Widget::Draw(ctx);
    }

    void SetDrawBackground(bool draw, glm::vec4 color = {0.15f, 0.15f, 0.18f, 0.95f}) {
        m_drawBackground = draw;
        m_bgColor = color;
        m_useThemeBg = false;
    }

    void SetDrawBackgroundFromTheme(bool draw) {
        m_drawBackground = draw;
        m_useThemeBg = true;
        if (draw) m_bgColor = ThemeManager::Get().GetTheme().panel.backgroundColor;
    }

    void ApplyTheme() override {
        if (m_drawBackground && m_useThemeBg)
            m_bgColor = ThemeManager::Get().GetTheme().panel.backgroundColor;
    }

    void SetPadding(float p) { m_padding = p; Recalculate(); }
    void SetSpacing(float s) { m_spacing = s; Recalculate(); }

    void Recalculate() {
        bool rtlHorizontal = (m_direction == BoxDirection::Horizontal && I18n::Get().IsRTL());

        if (rtlHorizontal) {
            float cursor = m_size.x - m_padding;
            for (auto* child : m_children) {
                cursor -= child->GetSize().x;
                child->SetPos({cursor, m_padding});
                cursor -= m_spacing;
            }
        } else {
            float cursor = m_padding;
            for (auto* child : m_children) {
                if (m_direction == BoxDirection::Vertical) {
                    child->SetPos({m_padding, cursor});
                    cursor += child->GetSize().y + m_spacing;
                } else {
                    child->SetPos({cursor, m_padding});
                    cursor += child->GetSize().x + m_spacing;
                }
            }
        }
    }

private:
    BoxDirection m_direction;
    float m_padding;
    float m_spacing;
    bool m_drawBackground = false;
    bool m_useThemeBg = false;
    glm::vec4 m_bgColor = {0.15f, 0.15f, 0.18f, 0.95f};
};

using VBoxLayout = BoxLayout;

class HBoxLayout : public BoxLayout {
public:
    HBoxLayout(int id, glm::vec2 pos, glm::vec2 size,
               float padding = 10.0f, float spacing = 8.0f)
        : BoxLayout(id, pos, size, BoxDirection::Horizontal, padding, spacing) {}
};

} // namespace CarHMI::GUI

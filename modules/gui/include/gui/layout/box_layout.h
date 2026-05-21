#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
#include <algorithm>

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

    void SetPadding(float p) { m_padding = Insets(p); Recalculate(); }
    void SetPadding(Insets p) { m_padding = p; Recalculate(); }
    Insets GetPadding() const { return m_padding; }

    void SetSpacing(float s) { m_spacing = s; Recalculate(); }
    float GetSpacing() const { return m_spacing; }

    void SetDirection(BoxDirection d) { m_direction = d; Recalculate(); }
    BoxDirection GetDirection() const { return m_direction; }

    void Recalculate() {
        if (m_children.empty()) return;

        float availW = m_size.x - m_padding.horizontal();
        float availH = m_size.y - m_padding.vertical();
        float totalSpacing = m_spacing * (float)(m_children.size() - 1);

        bool isVertical = (m_direction == BoxDirection::Vertical);

        // --- Pass 1: measure Fixed children on main axis, count Percent/Fill ---
        float fixedMain = 0;
        float totalPercent = 0;
        int fillCount = 0;

        for (auto* child : m_children) {
            auto sp = child->GetSizePolicy();
            auto margin = child->GetMargin();

            if (isVertical) {
                if (sp.heightMode == SizeMode::Fixed)
                    fixedMain += child->GetSize().y + margin.vertical();
                else if (sp.heightMode == SizeMode::Percent)
                    totalPercent += sp.heightValue;
                else
                    fillCount++;
            } else {
                if (sp.widthMode == SizeMode::Fixed)
                    fixedMain += child->GetSize().x + margin.horizontal();
                else if (sp.widthMode == SizeMode::Percent)
                    totalPercent += sp.widthValue;
                else
                    fillCount++;
            }
        }

        // Main-axis space left after Fixed children and spacing
        float mainAvail = isVertical ? availH : availW;
        float remainAfterFixed = std::max(0.0f, mainAvail - fixedMain - totalSpacing);

        // --- Pass 2: resolve sizes ---
        float usedByPercent = 0;

        for (auto* child : m_children) {
            auto sp = child->GetSizePolicy();
            auto margin = child->GetMargin();
            auto sz = child->GetSize();

            if (isVertical) {
                // Main axis (height)
                if (sp.heightMode == SizeMode::Percent) {
                    sz.y = remainAfterFixed * sp.heightValue / std::max(totalPercent, 100.0f);
                    usedByPercent += sz.y + margin.vertical();
                }
                // Cross axis (width)
                if (sp.widthMode == SizeMode::Percent)
                    sz.x = availW * sp.widthValue / 100.0f;
                else if (sp.widthMode == SizeMode::Fill)
                    sz.x = std::max(0.0f, availW - margin.horizontal());
            } else {
                // Main axis (width)
                if (sp.widthMode == SizeMode::Percent) {
                    sz.x = remainAfterFixed * sp.widthValue / std::max(totalPercent, 100.0f);
                    usedByPercent += sz.x + margin.horizontal();
                }
                // Cross axis (height)
                if (sp.heightMode == SizeMode::Percent)
                    sz.y = availH * sp.heightValue / 100.0f;
                else if (sp.heightMode == SizeMode::Fill)
                    sz.y = std::max(0.0f, availH - margin.vertical());
            }

            child->SetSize(sz);
        }

        // Distribute remaining to Fill children on main axis
        if (fillCount > 0) {
            float remainForFill = std::max(0.0f, remainAfterFixed - usedByPercent);
            float fillSize = remainForFill / (float)fillCount;

            for (auto* child : m_children) {
                auto sp = child->GetSizePolicy();
                auto margin = child->GetMargin();
                auto sz = child->GetSize();

                if (isVertical && sp.heightMode == SizeMode::Fill) {
                    sz.y = std::max(0.0f, fillSize - margin.vertical());
                    child->SetSize(sz);
                } else if (!isVertical && sp.widthMode == SizeMode::Fill) {
                    sz.x = std::max(0.0f, fillSize - margin.horizontal());
                    child->SetSize(sz);
                }
            }
        }

        // --- Pass 3: position children ---
        bool rtlHorizontal = (!isVertical && I18n::Get().IsRTL());

        if (rtlHorizontal) {
            float cursor = m_size.x - m_padding.right;
            for (auto* child : m_children) {
                auto margin = child->GetMargin();
                cursor -= margin.right;
                cursor -= child->GetSize().x;
                child->SetPos({cursor, m_padding.top + margin.top});
                cursor -= margin.left;
                cursor -= m_spacing;
            }
        } else if (isVertical) {
            float cursor = m_padding.top;
            for (auto* child : m_children) {
                auto margin = child->GetMargin();
                cursor += margin.top;
                child->SetPos({m_padding.left + margin.left, cursor});
                cursor += child->GetSize().y + margin.bottom + m_spacing;
            }
        } else {
            float cursor = m_padding.left;
            for (auto* child : m_children) {
                auto margin = child->GetMargin();
                cursor += margin.left;
                child->SetPos({cursor, m_padding.top + margin.top});
                cursor += child->GetSize().x + margin.right + m_spacing;
            }
        }
    }

private:
    BoxDirection m_direction;
    Insets m_padding;
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

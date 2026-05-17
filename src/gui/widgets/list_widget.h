#pragma once

#include "../widget.h"
#include "gui/style/theme_manager.h"
#include <spdlog/spdlog.h>
#include <functional>
#include <vector>
#include <string>

namespace CarHMI {

class ListWidget : public Widget {
public:
    ListWidget(int id, glm::vec2 pos, glm::vec2 size, float itemHeight = 40.0f)
        : Widget(id, pos, size), m_itemHeight(itemHeight) {
        m_focusable = true;
        m_propSelectedIndex = m_props.Register<float>("selectedIndex", -1.0f);
        ApplyTheme();
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().list;
        m_bgColor = s.bgColor;
        m_altRowColor = s.altRowColor;
        m_selectedColor = s.selectedColor;
        m_textColor = s.textColor;
        m_scrollbarColor = s.scrollbarColor;
    }

    void AddItem(const std::string& text) { m_items.push_back(text); }
    void ClearItems() { m_items.clear(); m_scrollOffset = 0; m_propSelectedIndex->Set(-1.0f); }

    void Update(UIContext& ctx) override {
        if (!m_visible || m_items.empty()) return;

        if (Contains(ctx.GetMousePos()) && ctx.IsMousePressed()) {
            glm::vec2 abs = GetAbsolutePos();
            float localY = ctx.GetMousePos().y - abs.y + m_scrollOffset;
            int clicked = (int)(localY / m_itemHeight);
            if (clicked >= 0 && clicked < (int)m_items.size()) {
                m_propSelectedIndex->Set((float)clicked);
                spdlog::info("List[{}] selected={} '{}'", m_id, clicked, m_items[clicked]);
                if (m_onItemSelected) m_onItemSelected(clicked);
            }
        }
    }

    void ScrollBy(float delta) {
        float maxScroll = glm::max(0.0f, m_items.size() * m_itemHeight - m_size.y);
        m_scrollOffset = glm::clamp(m_scrollOffset + delta, 0.0f, maxScroll);
    }

    void OnFocusAdjust(float delta) override {
        int idx = (int)m_propSelectedIndex->Get();
        idx += (delta > 0) ? 1 : -1;
        idx = glm::clamp(idx, 0, (int)m_items.size() - 1);
        m_propSelectedIndex->Set((float)idx);
        if (m_onItemSelected) m_onItemSelected(idx);
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        auto& renderer = ctx.GetRenderer();
        int selected = (int)m_propSelectedIndex->Get();

        renderer.DrawQuad(abs, m_size, m_bgColor);

        int firstVisible = (int)(m_scrollOffset / m_itemHeight);
        int visibleCount = (int)(m_size.y / m_itemHeight) + 2;

        for (int i = firstVisible; i < firstVisible + visibleCount && i < (int)m_items.size(); i++) {
            float itemY = abs.y + i * m_itemHeight - m_scrollOffset;
            if (itemY + m_itemHeight < abs.y || itemY > abs.y + m_size.y) continue;

            if (i == selected)
                renderer.DrawQuad({abs.x, itemY}, {m_size.x, m_itemHeight}, m_selectedColor);
            else if (i % 2 == 1)
                renderer.DrawQuad({abs.x, itemY}, {m_size.x, m_itemHeight}, m_altRowColor);

            float textY = itemY + (m_itemHeight - ctx.GetFont().GetLineHeight()) * 0.5f;
            ctx.GetFont().DrawText(renderer, m_items[i], abs.x + 12.0f, textY, m_textColor);
        }

        if (m_items.size() * m_itemHeight > m_size.y) {
            float totalHeight = m_items.size() * m_itemHeight;
            float barHeight = (m_size.y / totalHeight) * m_size.y;
            float barY = (m_scrollOffset / totalHeight) * m_size.y;
            renderer.DrawQuad({abs.x + m_size.x - 4.0f, abs.y + barY},
                              {4.0f, barHeight}, m_scrollbarColor);
        }

        DrawFocusHighlight(ctx);
    }

    Property<float>& SelectedIndexProperty() { return *m_propSelectedIndex; }
    void SetOnItemSelected(std::function<void(int)> cb) { m_onItemSelected = std::move(cb); }
    int GetSelectedIndex() const { return (int)m_propSelectedIndex->Get(); }
    const std::vector<std::string>& GetItems() const { return m_items; }

private:
    std::vector<std::string> m_items;
    float m_itemHeight;
    float m_scrollOffset = 0.0f;
    Property<float>* m_propSelectedIndex;
    std::function<void(int)> m_onItemSelected;

    glm::vec4 m_bgColor, m_altRowColor, m_selectedColor, m_textColor, m_scrollbarColor;
};

} // namespace CarHMI

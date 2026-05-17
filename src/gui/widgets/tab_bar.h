#pragma once

#include "../widget.h"
#include "gui/style/theme_manager.h"
#include "animation/animation_manager.h"
#include "animation/easing.h"
#include <spdlog/spdlog.h>
#include <functional>
#include <vector>
#include <string>

namespace CarHMI {

class TabBar : public Widget {
public:
    TabBar(int id, glm::vec2 pos, glm::vec2 size)
        : Widget(id, pos, size) {
        m_focusable = true;
        m_propActiveIndex = m_props.Register<float>("activeIndex", 0.0f);
        ApplyTheme();
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().tabBar;
        m_bgColor = s.bgColor;
        m_indicatorColor = s.indicatorColor;
        m_activeTextColor = s.activeTextColor;
        m_inactiveTextColor = s.inactiveTextColor;
    }

    void AddTab(const std::string& label) { m_tabs.push_back(label); }

    void Update(UIContext& ctx) override {
        if (!m_visible || m_tabs.empty()) return;

        float tabW = m_size.x / (float)m_tabs.size();

        if (Contains(ctx.GetMousePos()) && ctx.IsMousePressed()) {
            glm::vec2 abs = GetAbsolutePos();
            int clicked = (int)((ctx.GetMousePos().x - abs.x) / tabW);
            clicked = glm::clamp(clicked, 0, (int)m_tabs.size() - 1);
            SetActiveIndex(clicked);
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible || m_tabs.empty()) return;
        glm::vec2 abs = GetAbsolutePos();
        auto& renderer = ctx.GetRenderer();

        renderer.DrawQuad(abs, m_size, m_bgColor);

        float tabW = m_size.x / (float)m_tabs.size();
        int active = (int)m_propActiveIndex->Get();

        renderer.DrawQuad({abs.x + m_indicatorX, abs.y + m_size.y - 3.0f},
                          {tabW, 3.0f}, m_indicatorColor);

        for (int i = 0; i < (int)m_tabs.size(); i++) {
            glm::vec4 color = (i == active) ? m_activeTextColor : m_inactiveTextColor;
            float textW = ctx.GetFont().MeasureWidth(m_tabs[i]);
            float textH = ctx.GetFont().GetLineHeight();
            float tx = abs.x + tabW * i + (tabW - textW) * 0.5f;
            float ty = abs.y + (m_size.y - textH) * 0.5f - 2.0f;
            ctx.GetFont().DrawText(renderer, m_tabs[i], tx, ty, color);
        }
        DrawFocusHighlight(ctx);
    }

    void OnFocusAdjust(float delta) override {
        int idx = (int)m_propActiveIndex->Get();
        idx += (delta > 0) ? 1 : -1;
        idx = glm::clamp(idx, 0, (int)m_tabs.size() - 1);
        SetActiveIndex(idx);
    }

    void OnFocusActivate() override { FocusNext(); }

    Property<float>& ActiveIndexProperty() { return *m_propActiveIndex; }
    int GetActiveIndex() const { return (int)m_propActiveIndex->Get(); }

    void SetActiveIndex(int i) {
        i = glm::clamp(i, 0, (int)m_tabs.size() - 1);
        int prev = (int)m_propActiveIndex->Get();
        if (i == prev) return;

        m_propActiveIndex->Set((float)i);
        float tabW = m_size.x / (float)m_tabs.size();
        AnimationManager::Get().TweenTo(&m_indicatorX, tabW * i, 0.2f, Easing::OutCubic);
        spdlog::info("TabBar[{}] tab={} '{}'", m_id, i, m_tabs[i]);
        if (m_onTabChanged) m_onTabChanged(i);
    }

    void SetOnTabChanged(std::function<void(int)> cb) { m_onTabChanged = std::move(cb); }

private:
    void FocusNext() {
        int idx = ((int)m_propActiveIndex->Get() + 1) % (int)m_tabs.size();
        SetActiveIndex(idx);
    }

    std::vector<std::string> m_tabs;
    Property<float>* m_propActiveIndex;
    float m_indicatorX = 0.0f;
    std::function<void(int)> m_onTabChanged;

    glm::vec4 m_bgColor, m_indicatorColor, m_activeTextColor, m_inactiveTextColor;
};

} // namespace CarHMI

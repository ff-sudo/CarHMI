#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <core/animation/animation_manager.h>
#include <core/animation/easing.h>
#include <spdlog/spdlog.h>
#include <functional>

namespace CarHMI::GUI {

class Toggle : public Widget {
public:
    Toggle(int id, glm::vec2 pos, glm::vec2 size = {60, 30})
        : Widget(id, pos, size) {
        m_focusable = true;
        m_propOn = m_props.Register<float>("on", 0.0f);
        ApplyTheme();
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().toggle;
        m_offColor = s.offColor;
        m_onColor = s.onColor;
        m_handleColor = s.handleColor;
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;

        if (Contains(ctx.GetMousePos()) && ctx.IsMousePressed())
            DoToggle();
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        auto& renderer = ctx.GetRenderer();

        float h = m_size.y;
        float r = h * 0.5f;
        float slideT = m_propOn->Get();

        glm::vec4 trackColor = m_offColor + (m_onColor - m_offColor) * slideT;
        renderer.DrawQuad(abs, m_size, trackColor);
        renderer.DrawCircle(abs + glm::vec2(r, r), r, trackColor, 16);
        renderer.DrawCircle(abs + glm::vec2(m_size.x - r, r), r, trackColor, 16);

        float handleRadius = r * 0.8f;
        float handleX = abs.x + r + (m_size.x - 2 * r) * slideT;
        renderer.DrawCircle({handleX, abs.y + r}, handleRadius, m_handleColor, 16);
        DrawFocusHighlight(ctx);
    }

    void OnFocusActivate() override { DoToggle(); }

    Property<float>& OnProperty() { return *m_propOn; }
    bool IsOn() const { return m_propOn->Get() > 0.5f; }
    void SetOn(bool on) {
        m_on = on;
        AnimationManager::Get().TweenPropertyTo(*m_propOn, on ? 1.0f : 0.0f, 0.2f, Easing::OutCubic);
    }

    void SetOnChanged(std::function<void(bool)> cb) {
        m_onChanged = std::move(cb);
        m_onChangedConnection = m_propOn->OnChanged([this](const float& oldVal, const float& newVal) {
            bool wasOn = oldVal > 0.5f;
            bool isOn = newVal > 0.5f;
            if (wasOn != isOn && m_onChanged) m_onChanged(isOn);
        });
    }

private:
    void DoToggle() {
        m_on = !m_on;
        spdlog::info("Toggle[{}] = {}", m_id, m_on ? "ON" : "OFF");
        AnimationManager::Get().TweenPropertyTo(*m_propOn, m_on ? 1.0f : 0.0f, 0.2f, Easing::OutCubic);
        if (m_onChanged) m_onChanged(m_on);
    }

    bool m_on = false;
    Property<float>* m_propOn;
    std::function<void(bool)> m_onChanged;
    Core::Connection m_onChangedConnection;

    glm::vec4 m_offColor, m_onColor, m_handleColor;
};

} // namespace CarHMI::GUI

#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <spdlog/spdlog.h>
#include <functional>
#include <cmath>

namespace CarHMI::GUI {

class Knob : public Widget {
public:
    Knob(int id, glm::vec2 pos, float radius, float minVal = 0.0f, float maxVal = 100.0f)
        : Widget(id, pos, {radius * 2, radius * 2})
        , m_radius(radius), m_min(minVal), m_max(maxVal) {
        m_focusable = true;
        m_propValue = m_props.Register<float>("value", 0.0f);
        ApplyTheme();
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().knob;
        m_bgColor = s.bgColor;
        m_knobColor = s.knobColor;
        m_accentColor = s.accentColor;
        m_indicatorColor = s.indicatorColor;
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 center = GetAbsolutePos() + glm::vec2(m_radius);

        if (ctx.IsMousePressed()) {
            float dist = glm::length(ctx.GetMousePos() - center);
            if (dist <= m_radius) {
                ctx.SetActive(m_id);
                m_dragStartAngle = atan2f(ctx.GetMousePos().y - center.y,
                                          ctx.GetMousePos().x - center.x);
                m_dragStartValue = m_propValue->Get();
                spdlog::info("Knob[{}] grab", m_id);
            }
        }

        if (ctx.GetActive() == m_id) {
            if (ctx.IsMouseDown()) {
                float currentAngle = atan2f(ctx.GetMousePos().y - center.y,
                                            ctx.GetMousePos().x - center.x);
                float delta = currentAngle - m_dragStartAngle;
                if (delta > 3.14159f) delta -= 6.28318f;
                if (delta < -3.14159f) delta += 6.28318f;

                float sensitivity = (m_max - m_min) / 3.14159f;
                float newVal = glm::clamp(m_dragStartValue + delta * sensitivity, m_min, m_max);
                m_propValue->Set(newVal);
            } else {
                spdlog::info("Knob[{}] released, value={:.1f}", m_id, m_propValue->Get());
                ctx.ClearActive();
            }
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 center = GetAbsolutePos() + glm::vec2(m_radius);
        auto& renderer = ctx.GetRenderer();

        renderer.DrawCircle(center, m_radius, m_bgColor, 32);
        renderer.DrawCircle(center, m_radius * 0.85f, m_knobColor, 32);

        float t = (m_propValue->Get() - m_min) / (m_max - m_min);
        float valueAngle = 135.0f + t * 270.0f;
        if (t > 0.01f)
            renderer.DrawArc(center, m_radius * 0.88f, m_radius * 0.98f,
                             135.0f, valueAngle, m_accentColor, (int)(30 * t + 1));

        float indicatorAngle = glm::radians(valueAngle);
        glm::vec2 inner = center + glm::vec2(cosf(indicatorAngle), sinf(indicatorAngle)) * m_radius * 0.4f;
        glm::vec2 outer = center + glm::vec2(cosf(indicatorAngle), sinf(indicatorAngle)) * m_radius * 0.72f;
        renderer.DrawLine(inner, outer, 3.0f, m_indicatorColor);
        DrawFocusHighlight(ctx);
    }

    void OnFocusAdjust(float delta) override {
        float step = (m_max - m_min) * 0.03f;
        m_propValue->Set(glm::clamp(m_propValue->Get() + delta * step, m_min, m_max));
    }

    Property<float>& ValueProperty() { return *m_propValue; }
    void SetOnChanged(std::function<void(float)> cb) {
        m_propValue->OnChanged([cb](const float&, const float& v) { cb(v); });
    }
    void SetValue(float v) { m_propValue->Set(glm::clamp(v, m_min, m_max)); }
    float GetValue() const { return m_propValue->Get(); }

private:
    float m_radius, m_min, m_max;
    float m_dragStartAngle = 0.0f, m_dragStartValue = 0.0f;
    Property<float>* m_propValue;

    glm::vec4 m_bgColor, m_knobColor, m_accentColor, m_indicatorColor;
};

} // namespace CarHMI::GUI

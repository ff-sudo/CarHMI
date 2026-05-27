#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <string>
#include <cstdio>
#include <cmath>

namespace CarHMI::GUI {

class Gauge : public Widget {
public:
    Gauge(int id, glm::vec2 pos, float radius, float minVal = 0.0f, float maxVal = 240.0f)
        : Widget(id, pos, {radius * 2, radius * 2})
        , m_radius(radius), m_min(minVal), m_max(maxVal) {
        m_propValue = m_props.Register<float>("value", 0.0f);
        ApplyTheme();
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().gauge;
        m_bgColor = s.bgColor;
        m_trackColor = s.trackColor;
        m_fillColor = s.fillColor;
        m_warningColor = s.warningColor;
        m_dangerColor = s.dangerColor;
        m_needleColor = s.needleColor;
        m_textColor = s.textColor;
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        glm::vec2 center = abs + glm::vec2(m_radius, m_radius);
        auto& renderer = ctx.GetRenderer();

        // background circle
        renderer.DrawCircle(center, m_radius, m_bgColor, 48);

        // track arc (135° to 405°)
        renderer.DrawArc(center, m_radius * 0.75f, m_radius * 0.85f,
                         m_startAngle, m_endAngle, m_trackColor, 40);

        // value arc
        float t = (m_propValue->Get() - m_min) / (m_max - m_min);
        float valueAngle = m_startAngle + t * (m_endAngle - m_startAngle);
        if (t > 0.01f) {
            glm::vec4 fillColor = m_fillColor;
            if (t > 0.85f) fillColor = m_dangerColor;
            else if (t > 0.6f) fillColor = m_warningColor;
            renderer.DrawArc(center, m_radius * 0.75f, m_radius * 0.85f,
                             m_startAngle, valueAngle, fillColor, (int)(40 * t + 1));
        }

        // needle
        float needleAngle = glm::radians(valueAngle);
        float needleLen = m_radius * 0.65f;
        glm::vec2 needleTip = center + glm::vec2(cosf(needleAngle), sinf(needleAngle)) * needleLen;
        renderer.DrawLine(center, needleTip, 3.0f, m_needleColor);

        // center dot
        renderer.DrawCircle(center, m_radius * 0.08f, m_needleColor, 16);

        // value text
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", m_propValue->Get());
        float textW = ctx.GetFont().MeasureWidth(buf);
        ctx.GetFont().DrawText(renderer, buf, center.x - textW * 0.5f,
                               center.y + m_radius * 0.25f, m_textColor);

        // unit label
        if (!m_unit.empty()) {
            float unitW = ctx.GetFont().MeasureWidth(m_unit);
            ctx.GetFont().DrawText(renderer, m_unit, center.x - unitW * 0.5f,
                                   center.y + m_radius * 0.45f, m_trackColor);
        }
    }

    void SetValue(float v) { m_propValue->Set(glm::clamp(v, m_min, m_max)); }
    float GetValue() const { return m_propValue->Get(); }
    Property<float>& ValueProperty() { return *m_propValue; }
    void SetUnit(const std::string& u) { m_unit = u; }

    void SetColors(const glm::vec4& bg, const glm::vec4& fill, const glm::vec4& needle) {
        m_bgColor = bg; m_fillColor = fill; m_needleColor = needle;
    }

private:
    float m_radius;
    float m_min, m_max;
    Property<float>* m_propValue;
    std::string m_unit = "km/h";

    float m_startAngle = 135.0f;
    float m_endAngle = 405.0f;

    glm::vec4 m_bgColor;
    glm::vec4 m_trackColor;
    glm::vec4 m_fillColor;
    glm::vec4 m_warningColor;
    glm::vec4 m_dangerColor;
    glm::vec4 m_needleColor;
    glm::vec4 m_textColor;
};

} // namespace CarHMI::GUI

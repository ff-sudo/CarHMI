#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
#include <spdlog/spdlog.h>
#include <functional>

namespace CarHMI::GUI {

class Slider : public Widget {
public:
    Slider(int id, glm::vec2 pos, glm::vec2 size,
           float minVal = 0.0f, float maxVal = 1.0f)
        : Widget(id, pos, size), m_min(minVal), m_max(maxVal) {
        m_focusable = true;
        m_propValue = m_props.Register<float>("value", minVal);
        ApplyTheme();
    }

    void ApplyTheme() {
        auto& s = ThemeManager::Get().GetTheme().slider;
        m_trackColor = s.trackColor;
        m_fillColor = s.fillColor;
        m_handleColor = s.handleColor;
        m_handleActive = s.handleActive;
        m_trackHeight = s.trackHeight;
        m_handleWidth = s.handleWidth;
        m_handleHeight = s.handleHeight;
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;

        if (Contains(ctx.GetMousePos()) && ctx.IsMousePressed()) {
            ctx.SetActive(m_id);
            spdlog::info("Slider[{}] grab started", m_id);
        }

        if (ctx.GetActive() == m_id) {
            if (ctx.IsMouseDown()) {
                glm::vec2 abs = GetAbsolutePos();
                float t = (ctx.GetMousePos().x - abs.x) / m_size.x;
                t = glm::clamp(t, 0.0f, 1.0f);
                if (I18n::Get().IsRTL()) t = 1.0f - t;
                float newVal = m_min + t * (m_max - m_min);
                m_propValue->Set(newVal);
            } else {
                spdlog::info("Slider[{}] released, value={:.1f}", m_id, m_propValue->Get());
                ctx.ClearActive();
            }
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();

        float trackY = abs.y + (m_size.y - m_trackHeight) * 0.5f;
        ctx.GetRenderer().DrawQuad({abs.x, trackY}, {m_size.x, m_trackHeight}, m_trackColor);

        float t = (m_propValue->Get() - m_min) / (m_max - m_min);
        bool rtl = I18n::Get().IsRTL();
        float visualT = rtl ? (1.0f - t) : t;

        if (rtl)
            ctx.GetRenderer().DrawQuad({abs.x + m_size.x * visualT, trackY},
                                       {m_size.x * t, m_trackHeight}, m_fillColor);
        else
            ctx.GetRenderer().DrawQuad({abs.x, trackY}, {m_size.x * t, m_trackHeight}, m_fillColor);

        float handleX = abs.x + m_size.x * visualT - m_handleWidth * 0.5f;
        float handleY = abs.y + (m_size.y - m_handleHeight) * 0.5f;
        glm::vec4 hColor = (ctx.GetActive() == m_id) ? m_handleActive : m_handleColor;
        ctx.GetRenderer().DrawQuad({handleX, handleY}, {m_handleWidth, m_handleHeight}, hColor);
        DrawFocusHighlight(ctx);
    }

    void OnFocusAdjust(float delta) override {
        float step = (m_max - m_min) * 0.02f;
        float newVal = glm::clamp(m_propValue->Get() + delta * step, m_min, m_max);
        m_propValue->Set(newVal);
    }

    Property<float>& ValueProperty() { return *m_propValue; }
    void SetValue(float v) { m_propValue->Set(glm::clamp(v, m_min, m_max)); }
    float GetValue() const { return m_propValue->Get(); }

    // Legacy callback bridge
    void SetOnChanged(std::function<void(float)> cb) {
        m_propValue->OnChanged([cb](const float&, const float& newVal) {
            cb(newVal);
        });
    }

private:
    float m_min, m_max;
    Property<float>* m_propValue;

    glm::vec4 m_trackColor, m_fillColor, m_handleColor, m_handleActive;
    float m_trackHeight, m_handleWidth, m_handleHeight;
};

} // namespace CarHMI::GUI

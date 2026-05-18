#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>

namespace CarHMI::GUI {

class ProgressBar : public Widget {
public:
    ProgressBar(int id, glm::vec2 pos, glm::vec2 size)
        : Widget(id, pos, size) {
        m_propValue = m_props.Register<float>("value", 0.0f);
        ApplyTheme();
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().progressBar;
        m_themeBgColor = s.backgroundColor;
        m_themeFillColor = s.fillColor;
        if (!m_customColors) {
            m_bgColor = m_themeBgColor;
            m_fgColor = m_themeFillColor;
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();

        ctx.GetRenderer().DrawQuad(abs, m_size, m_bgColor);

        float fillW = m_size.x * glm::clamp(m_propValue->Get(), 0.0f, 1.0f);
        if (fillW > 0) {
            float fillX = I18n::Get().IsRTL() ? (abs.x + m_size.x - fillW) : abs.x;
            ctx.GetRenderer().DrawQuad({fillX, abs.y}, {fillW, m_size.y}, m_fgColor);
        }
    }

    Property<float>& ValueProperty() { return *m_propValue; }
    void SetValue(float v) { m_propValue->Set(glm::clamp(v, 0.0f, 1.0f)); }
    float GetValue() const { return m_propValue->Get(); }

    void SetColors(const glm::vec4& fg, const glm::vec4& bg) {
        m_fgColor = fg; m_bgColor = bg; m_customColors = true;
    }

    void ResetToThemeColors() {
        m_customColors = false;
        m_fgColor = m_themeFillColor;
        m_bgColor = m_themeBgColor;
    }

private:
    Property<float>* m_propValue;
    bool m_customColors = false;
    glm::vec4 m_fgColor, m_bgColor;
    glm::vec4 m_themeFillColor, m_themeBgColor;
};

} // namespace CarHMI::GUI

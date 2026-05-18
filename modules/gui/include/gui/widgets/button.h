#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
#include <core/animation/animation_manager.h>
#include <core/animation/tween.h>
#include <core/animation/easing.h>
#include <core/event_bus.h>
#include <core/connection.h>
#include <spdlog/spdlog.h>
#include <string>
#include <functional>

namespace CarHMI::GUI {

class Button : public Widget {
public:
    Button(int id, glm::vec2 pos, glm::vec2 size, const std::string& text)
        : Widget(id, pos, size) {
        m_focusable = true;
        m_propText = m_props.Register<std::string>("text", text);
        ApplyTheme();
    }

    void SetI18nKey(const std::string& key) {
        m_i18nKey = key;
        m_propText->Set(I18n::Get().T(key));
        if (!m_langSubscribed) {
            m_langSubscribed = true;
            m_langConnection = Core::EventBus::Get().Subscribe<LanguageChangedEvent>(
                [this](const LanguageChangedEvent&) {
                    if (!m_i18nKey.empty())
                        m_propText->Set(I18n::Get().T(m_i18nKey));
                });
        }
    }

    void ApplyTheme() override {
        auto& s = ThemeManager::Get().GetTheme().button;
        m_normalColor = s.normalColor;
        m_hoverColor = s.hoverColor;
        m_pressColor = s.pressColor;
        m_textColor = s.textColor;
        m_transitionTime = s.transitionTime;
        m_currentColor = m_normalColor;
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;

        State prevState = m_state;
        m_state = State::Normal;

        if (Contains(ctx.GetMousePos())) {
            ctx.SetHot(m_id);
            m_state = State::Hovered;

            if (ctx.IsMousePressed()) {
                ctx.SetActive(m_id);
                m_state = State::Pressed;
                spdlog::info("Button[{}] '{}' pressed", m_id, m_propText->Get());
            }

            if (ctx.IsMouseReleased() && ctx.GetActive() == m_id) {
                spdlog::info("Button[{}] '{}' clicked!", m_id, m_propText->Get());
                if (m_onClick) m_onClick();
                ctx.ClearActive();
            }
        }

        if (m_state != prevState) {
            glm::vec4 targetColor;
            switch (m_state) {
                case State::Hovered: targetColor = m_hoverColor; break;
                case State::Pressed: targetColor = m_pressColor; break;
                default:             targetColor = m_normalColor; break;
            }
            AnimationManager::Get().TweenTo(&m_currentColor, targetColor, m_transitionTime, Easing::OutQuad);
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();

        ctx.GetRenderer().DrawQuad(abs, m_size, m_currentColor);

        float textW = ctx.GetFont().MeasureWidth(m_propText->Get());
        float textH = ctx.GetFont().GetLineHeight();
        float tx = abs.x + (m_size.x - textW) * 0.5f;
        float ty = abs.y + (m_size.y - textH) * 0.5f;
        ctx.GetFont().DrawText(ctx.GetRenderer(), m_propText->Get(), tx, ty, m_textColor);
        DrawFocusHighlight(ctx);
    }

    void OnFocusActivate() override {
        spdlog::info("Button[{}] '{}' focus-activated", m_id, m_propText->Get());
        if (m_onClick) m_onClick();
    }

    Property<std::string>& TextProperty() { return *m_propText; }
    void SetOnClick(std::function<void()> cb) { m_onClick = std::move(cb); }
    void SetText(const std::string& t) { m_i18nKey.clear(); m_propText->Set(t); }

private:
    enum class State { Normal, Hovered, Pressed };

    Property<std::string>* m_propText;
    State m_state = State::Normal;
    std::function<void()> m_onClick;

    glm::vec4 m_normalColor, m_hoverColor, m_pressColor;
    glm::vec4 m_currentColor;
    glm::vec4 m_textColor;
    float m_transitionTime = 0.15f;

    std::string m_i18nKey;
    Core::Connection m_langConnection;
    bool m_langSubscribed = false;
};

} // namespace CarHMI::GUI

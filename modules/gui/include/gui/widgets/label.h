#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
#include <core/event_bus.h>
#include <core/connection.h>
#include <string>

namespace CarHMI::GUI {

class Label : public Widget {
public:
    enum class Role { Text, Title, Subtitle, Custom };

    Label(int id, glm::vec2 pos, const std::string& text, Role role = Role::Text)
        : Widget(id, pos, {0, 0}), m_role(role) {
        m_propText = m_props.Register<std::string>("text", text);
        ApplyTheme();
    }

    Label(int id, glm::vec2 pos, const std::string& text, glm::vec4 customColor)
        : Widget(id, pos, {0, 0}), m_role(Role::Custom), m_color(customColor) {
        m_propText = m_props.Register<std::string>("text", text);
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
        auto& s = ThemeManager::Get().GetTheme().label;
        switch (m_role) {
            case Role::Text:     m_color = s.textColor; break;
            case Role::Title:    m_color = s.titleColor; break;
            case Role::Subtitle: m_color = s.subtitleColor; break;
            case Role::Custom:   break;
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        ctx.GetFont().DrawText(ctx.GetRenderer(), m_propText->Get(), abs.x, abs.y, m_color);
    }

    Property<std::string>& TextProperty() { return *m_propText; }
    void SetText(const std::string& t) { m_i18nKey.clear(); m_propText->Set(t); }
    void SetColor(const glm::vec4& c) { m_color = c; m_role = Role::Custom; }
    void SetRole(Role r) { m_role = r; ApplyTheme(); }
    const std::string& GetText() const { return m_propText->Get(); }

private:
    Property<std::string>* m_propText;
    glm::vec4 m_color = {1, 1, 1, 1};
    Role m_role;
    std::string m_i18nKey;
    Core::Connection m_langConnection;
    bool m_langSubscribed = false;
};

} // namespace CarHMI::GUI

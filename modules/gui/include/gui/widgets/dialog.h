#pragma once

#include <gui/widget.h>
#include <gui/widgets/label.h>
#include <gui/widgets/button.h>
#include <gui/layout/box_layout.h>
#include <gui/style/theme_manager.h>
#include <core/animation/animation_manager.h>
#include <core/animation/easing.h>
#include <core/timer/timer_manager.h>
#include <functional>
#include <string>
#include <algorithm>

namespace CarHMI::GUI {

// ============================================================
// Dialog — 模态弹窗
// ============================================================

class Dialog : public Widget {
public:
    Dialog(int id, glm::vec2 pos, glm::vec2 size,
           const std::string& title = "Dialog",
           const std::string& message = "")
        : Widget(id, pos, size) {
        BuildUI(title, message);
    }

    void AddButton(const std::string& text, std::function<void()> onClick, bool /*isPrimary*/ = false) {
        int btnId = m_id * 100 + (int)m_buttons.size() + 1;
        auto* btn = new Button(btnId, {0, 0}, {80, 36}, text);
        btn->SetOnClick(onClick);
        m_buttons.push_back(btn);
        m_buttonRow->AddChild(btn);

        // Evenly distribute button widths
        float btnW = (m_size.x - 40.0f - m_spacing * (float)(m_buttons.size() - 1)) / (float)m_buttons.size();
        btnW = std::max(btnW, 60.0f);
        for (auto* b : m_buttons)
            b->SetSize({btnW, 36});
    }

    void SetTitle(const std::string& title) {
        m_titleLabel->SetText(title);
    }

    void SetMessage(const std::string& msg) {
        m_messageLabel->SetText(msg);
    }

    void SetTitleI18n(const std::string& key) {
        m_titleLabel->SetI18nKey(key);
    }

    void SetMessageI18n(const std::string& key) {
        m_messageLabel->SetI18nKey(key);
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;

        glm::vec2 abs = GetAbsolutePos();
        auto& renderer = ctx.GetRenderer();

        // Background
        float radius = m_radius > 0 ? m_radius : ThemeManager::Get().GetTheme().widget.borderRadius;
        if (radius > 0)
            renderer.DrawRoundedRect(abs, m_size, radius, m_bgColor);
        else
            renderer.DrawQuad(abs, m_size, m_bgColor);

        // Title separator line
        float titleH = 40.0f;
        glm::vec4 sepColor = m_bgColor + glm::vec4(0.08f, 0.08f, 0.08f, 0.0f);
        renderer.DrawQuad({abs.x + m_padding, abs.y + titleH},
                          {m_size.x - m_padding * 2, 1.0f}, sepColor);

        Widget::Draw(ctx);
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;
        Widget::Update(ctx);
    }

    void SetCornerRadius(float r) { m_radius = r; }

private:
    void BuildUI(const std::string& title, const std::string& message) {
        auto& theme = ThemeManager::Get().GetTheme();
        m_bgColor = theme.panel.backgroundColor;
        m_bgColor.a = 0.98f;
        m_padding = theme.widget.padding;
        m_spacing = theme.widget.spacing;

        m_titleLabel = new Label(m_id * 100 + 1, {m_padding, (40.0f - 18.0f) * 0.5f},
                                 title, Label::Role::Title);

        float msgY = 50.0f;
        m_messageLabel = new Label(m_id * 100 + 2, {m_padding, msgY},
                                   message, Label::Role::Text);
        m_messageLabel->SetSize({m_size.x - m_padding * 2, m_size.y - msgY - 60.0f});

        m_buttonRow = new HBoxLayout(m_id * 100 + 3,
                                     {m_padding, m_size.y - 50.0f},
                                     {m_size.x - m_padding * 2, 36.0f}, 0, m_spacing);

        AddChild(m_titleLabel);
        AddChild(m_messageLabel);
        AddChild(m_buttonRow);
    }

    Label* m_titleLabel;
    Label* m_messageLabel;
    HBoxLayout* m_buttonRow;
    std::vector<Button*> m_buttons;

    glm::vec4 m_bgColor;
    float m_radius = 0.0f;
    float m_padding = 10.0f;
    float m_spacing = 8.0f;
};

// ============================================================
// Toast — 通知提示
// ============================================================

enum class ToastType { Info, Success, Warning, Error };

class Toast : public Widget {
public:
    Toast(int id, glm::vec2 pos, glm::vec2 size, const std::string& message = "")
        : Widget(id, pos, size) {
        m_message = message;
    }

    void SetMessage(const std::string& msg) { m_message = msg; }
    void SetDuration(float seconds) { m_duration = seconds; }
    void SetType(ToastType type) { m_toastType = type; }

    void Show() {
        if (!m_visible) {
            m_visible = true;
            m_elapsed = 0.0f;
            m_opacity = 0.0f;
            m_offsetY = -20.0f;

            // Animate in
            Core::AnimationManager::Get().TweenTo(&m_opacity, 1.0f, 0.25f, Core::Easing::OutCubic);
            Core::AnimationManager::Get().TweenTo(&m_offsetY, 0.0f, 0.25f, Core::Easing::OutCubic);

            // Auto-dismiss
            m_dismissTimer = Core::TimerManager::Get().SetTimeout([this]() { Hide(); },
                                                            (int)(m_duration * 1000));
        }
    }

    void Hide() {
        m_dismissTimer.Disconnect();
        auto* tween = Core::AnimationManager::Get().TweenTo(&m_opacity, 0.0f, 0.2f, Core::Easing::InCubic);
        tween->SetOnComplete([this]() { m_visible = false; });
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();

        // Slide offset
        glm::vec2 drawPos = abs + glm::vec2(0, m_offsetY);

        // Background
        glm::vec4 bgColor = GetToastColor();
        bgColor.a *= m_opacity;
        ctx.GetRenderer().DrawRoundedRect(drawPos, m_size, 6.0f, bgColor);

        // Message text
        glm::vec4 textColor = {1.0f, 1.0f, 1.0f, m_opacity};
        float textW = ctx.GetFont().MeasureWidth(m_message);
        float textH = ctx.GetFont().GetLineHeight();
        float tx = drawPos.x + (m_size.x - textW) * 0.5f;
        float ty = drawPos.y + (m_size.y - textH) * 0.5f;
        ctx.GetFont().DrawText(ctx.GetRenderer(), m_message, tx, ty, textColor);
    }

private:
    glm::vec4 GetToastColor() const {
        switch (m_toastType) {
        case ToastType::Success: return {0.1f, 0.6f, 0.3f, 0.92f};
        case ToastType::Warning: return {0.9f, 0.6f, 0.1f, 0.92f};
        case ToastType::Error:   return {0.9f, 0.2f, 0.2f, 0.92f};
        case ToastType::Info:
        default:                 return {0.2f, 0.25f, 0.35f, 0.92f};
        }
    }

    std::string m_message;
    ToastType m_toastType = ToastType::Info;
    float m_duration = 3.0f;
    float m_elapsed = 0.0f;
    float m_opacity = 0.0f;
    float m_offsetY = 0.0f;
    Core::Connection m_dismissTimer;
};

} // namespace CarHMI::GUI

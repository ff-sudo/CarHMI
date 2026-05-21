#pragma once

#include <gui/widget.h>
#include <gui/focus_manager.h>
#include <gui/style/theme_manager.h>
#include <core/event_bus.h>
#include <core/event.h>
#include <SDL.h>
#include <string>
#include <functional>
#include <algorithm>
#include <cstring>

namespace CarHMI::GUI {

class TextInput : public Widget {
public:
    TextInput(int id, glm::vec2 pos, glm::vec2 size,
              const std::string& placeholder = "")
        : Widget(id, pos, size), m_placeholder(placeholder) {
        m_focusable = true;

        m_connections.Add(Core::EventBus::Get().Subscribe<Core::TextInputEvent>(
            [this](const Core::TextInputEvent& e) {
                if (!IsFocused()) return;
                InsertText(e.text);
            }));

        ApplyTheme();
    }

    void SetText(const std::string& text) {
        m_text = text;
        m_cursorPos = (int)CountChars(m_text);
        ClampCursor();
        if (m_onTextChanged) m_onTextChanged(m_text);
    }

    std::string GetText() const { return m_text; }

    void SetPlaceholder(const std::string& text) { m_placeholder = text; }

    void SetOnTextChanged(std::function<void(const std::string&)> cb) {
        m_onTextChanged = std::move(cb);
    }

    void SetOnSubmit(std::function<void(const std::string&)> cb) {
        m_onSubmit = std::move(cb);
    }

    void ApplyTheme() override {
        auto& theme = ThemeManager::Get().GetTheme();
        m_bgColor = {0.1f, 0.1f, 0.13f, 1.0f};
        m_borderColor = theme.button.normalColor;
        m_textColor = theme.label.textColor;
        m_placeholderColor = {0.5f, 0.5f, 0.55f, 1.0f};
        m_focusBorderColor = {0.2f, 0.6f, 0.9f, 1.0f};
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;

        bool focused = IsFocused();

        // Click to focus and position cursor
        if (ctx.IsMousePressed() && Contains(ctx.GetMousePos())) {
            // Find focus index and set
            auto& fm = FocusManager::Get();
            auto& chain = fm.GetFocusChain();
            for (int i = 0; i < (int)chain.size(); i++) {
                if (chain[i] == this) {
                    fm.SetFocusIndex(i);
                    break;
                }
            }
            // Position cursor based on click X
            PositionCursorFromClick(ctx);
        }

        // Cursor blink
        if (focused) {
            m_blinkTimer += 1.0f / 60.0f; // approximate
            if (m_blinkTimer >= 0.5f) {
                m_blinkTimer -= 0.5f;
                m_cursorBlink = !m_cursorBlink;
            }
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        bool focused = IsFocused();

        float padding = 6.0f;

        // Background
        ctx.GetRenderer().DrawRoundedRect(abs, m_size, 4.0f, m_bgColor);

        // Border
        glm::vec4 border = focused ? m_focusBorderColor : m_borderColor;
        float bw = focused ? 2.0f : 1.0f;
        // Top
        ctx.GetRenderer().DrawQuad(abs, {m_size.x, bw}, border);
        // Bottom
        ctx.GetRenderer().DrawQuad({abs.x, abs.y + m_size.y - bw}, {m_size.x, bw}, border);
        // Left
        ctx.GetRenderer().DrawQuad(abs, {bw, m_size.y}, border);
        // Right
        ctx.GetRenderer().DrawQuad({abs.x + m_size.x - bw, abs.y}, {bw, m_size.y}, border);

        // Clip text area
        float textAreaW = m_size.x - padding * 2;
        ctx.PushClipRect({abs.x + padding, abs.y}, {textAreaW, m_size.y});

        float textY = abs.y + (m_size.y - ctx.GetFont().GetLineHeight()) * 0.5f;
        float textX = abs.x + padding - m_scrollOffsetX;

        if (m_text.empty() && !focused) {
            // Placeholder
            ctx.GetFont().DrawText(ctx.GetRenderer(), m_placeholder,
                                   textX, textY, m_placeholderColor);
        } else {
            // Text
            ctx.GetFont().DrawText(ctx.GetRenderer(), m_text,
                                   textX, textY, m_textColor);
        }

        // Cursor
        if (focused && m_cursorBlink) {
            float cursorX = textX + ctx.GetFont().MeasureSubstring(m_text, m_cursorPos);
            float cursorY1 = abs.y + 4.0f;
            float cursorY2 = abs.y + m_size.y - 4.0f;
            ctx.GetRenderer().DrawLine({cursorX, cursorY1}, {cursorX, cursorY2},
                                       1.5f, m_textColor);
        }

        ctx.PopClipRect();
    }

    bool OnKeyEvent(const Core::KeyEvent& e) override {
        if (!IsFocused() || !e.pressed) return false;

        switch (e.scancode) {
        case SDL_SCANCODE_BACKSPACE:
            if (m_cursorPos > 0) {
                EraseCharAt(m_cursorPos - 1);
                m_cursorPos--;
                OnTextModified();
            }
            return true;
        case SDL_SCANCODE_DELETE:
            if (m_cursorPos < (int)CountChars(m_text)) {
                EraseCharAt(m_cursorPos);
                OnTextModified();
            }
            return true;
        case SDL_SCANCODE_LEFT:
            if (m_cursorPos > 0) { m_cursorPos--; ResetBlink(); }
            return true;
        case SDL_SCANCODE_RIGHT:
            if (m_cursorPos < (int)CountChars(m_text)) { m_cursorPos++; ResetBlink(); }
            return true;
        case SDL_SCANCODE_HOME:
            m_cursorPos = 0; ResetBlink();
            return true;
        case SDL_SCANCODE_END:
            m_cursorPos = (int)CountChars(m_text); ResetBlink();
            return true;
        case SDL_SCANCODE_RETURN:
            if (m_onSubmit) m_onSubmit(m_text);
            return true;
        default:
            return false;
        }
    }

    void OnFocusActivate() override {
        if (m_onSubmit) m_onSubmit(m_text);
    }

private:
    bool IsFocused() const {
        return FocusManager::Get().IsFocused(m_id);
    }

    void InsertText(const char* text) {
        std::string ins(text);
        int bytePos = CharIndexToByteIndex(m_text, m_cursorPos);
        m_text.insert(bytePos, ins);
        m_cursorPos += (int)CountChars(ins);
        OnTextModified();
    }

    void OnTextModified() {
        ResetBlink();
        EnsureCursorVisible();
        if (m_onTextChanged) m_onTextChanged(m_text);
    }

    void ResetBlink() {
        m_blinkTimer = 0;
        m_cursorBlink = true;
    }

    void ClampCursor() {
        int len = (int)CountChars(m_text);
        m_cursorPos = std::clamp(m_cursorPos, 0, len);
    }

    void EnsureCursorVisible() {
        // Need a font reference for measurement — we'll use a rough approach
        // The actual scroll adjustment happens in Draw, but we pre-compute here
        // This is deferred to Draw since we don't have font ref in non-draw context
    }

    void PositionCursorFromClick(UIContext& ctx) {
        glm::vec2 abs = GetAbsolutePos();
        float padding = 6.0f;
        float clickX = ctx.GetMousePos().x - abs.x - padding + m_scrollOffsetX;

        int charCount = (int)CountChars(m_text);
        int best = 0;
        float bestDist = clickX; // distance from char 0

        for (int i = 1; i <= charCount; i++) {
            float w = ctx.GetFont().MeasureSubstring(m_text, i);
            float dist = std::abs(clickX - w);
            if (dist < bestDist) {
                bestDist = dist;
                best = i;
            }
        }
        m_cursorPos = best;
        ResetBlink();
    }

    // --- UTF-8 helpers ---

    static size_t CountChars(const std::string& s) {
        size_t count = 0;
        const char* p = s.c_str();
        const char* end = p + s.size();
        while (p < end) {
            unsigned char c = *p;
            if (c < 0x80) p += 1;
            else if (c < 0xE0) p += 2;
            else if (c < 0xF0) p += 3;
            else p += 4;
            count++;
        }
        return count;
    }

    static int CharIndexToByteIndex(const std::string& s, int charIdx) {
        const char* p = s.c_str();
        const char* end = p + s.size();
        int ci = 0;
        while (p < end && ci < charIdx) {
            unsigned char c = *p;
            if (c < 0x80) p += 1;
            else if (c < 0xE0) p += 2;
            else if (c < 0xF0) p += 3;
            else p += 4;
            ci++;
        }
        return (int)(p - s.c_str());
    }

    void EraseCharAt(int charIdx) {
        int byteStart = CharIndexToByteIndex(m_text, charIdx);
        int byteEnd = CharIndexToByteIndex(m_text, charIdx + 1);
        m_text.erase(byteStart, byteEnd - byteStart);
    }

    // --- State ---
    std::string m_text;
    std::string m_placeholder;
    int m_cursorPos = 0;
    float m_scrollOffsetX = 0;
    float m_blinkTimer = 0;
    bool m_cursorBlink = true;

    glm::vec4 m_bgColor;
    glm::vec4 m_borderColor;
    glm::vec4 m_textColor;
    glm::vec4 m_placeholderColor;
    glm::vec4 m_focusBorderColor;

    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onSubmit;

    Core::ConnectionGroup m_connections;
};

} // namespace CarHMI::GUI

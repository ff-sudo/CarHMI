#pragma once

#include <gui/widget.h>
#include <gui/focus_manager.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
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
                DeleteSelection();
                InsertText(e.text);
            }));

        ApplyTheme();
    }

    void SetText(const std::string& text) {
        m_text = text;
        m_cursorPos = (int)CountChars(m_text);
        ClearSelection();
        ClampCursor();
        if (m_onTextChanged) m_onTextChanged(m_text);
    }

    std::string GetText() const { return m_text; }

    void SetPlaceholder(const std::string& text) { m_placeholder = text; }

    void SetPlaceholderI18nKey(const std::string& key) {
        m_placeholderI18nKey = key;
        m_placeholder = I18n::Get().T(key);
        if (!m_langSubscribed) {
            m_langSubscribed = true;
            m_connections.Add(Core::EventBus::Get().Subscribe<LanguageChangedEvent>(
                [this](const LanguageChangedEvent&) {
                    if (!m_placeholderI18nKey.empty())
                        m_placeholder = I18n::Get().T(m_placeholderI18nKey);
                }));
        }
    }

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
        m_focusBorderColor = theme.widget.accentColor;
    }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;

        bool focused = IsFocused();

        // Click to focus and position cursor
        if (ctx.IsMousePressed() && Contains(ctx.GetMousePos())) {
            auto& fm = FocusManager::Get();
            auto& chain = fm.GetFocusChain();
            for (int i = 0; i < (int)chain.size(); i++) {
                if (chain[i] == this) {
                    fm.SetFocusIndex(i);
                    break;
                }
            }
            PositionCursorFromClick(ctx);
            ClearSelection();
        }

        // Cursor blink
        if (focused) {
            m_blinkTimer += ctx.GetDeltaTime();
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
        ctx.GetRenderer().DrawQuad(abs, {m_size.x, bw}, border);
        ctx.GetRenderer().DrawQuad({abs.x, abs.y + m_size.y - bw}, {m_size.x, bw}, border);
        ctx.GetRenderer().DrawQuad(abs, {bw, m_size.y}, border);
        ctx.GetRenderer().DrawQuad({abs.x + m_size.x - bw, abs.y}, {bw, m_size.y}, border);

        // Ensure cursor visible (scroll adjustment)
        float textAreaW = m_size.x - padding * 2;
        EnsureCursorVisible(ctx, textAreaW);

        // Clip text area
        ctx.PushClipRect({abs.x + padding, abs.y}, {textAreaW, m_size.y});

        float textY = abs.y + (m_size.y - ctx.GetFont().GetLineHeight()) * 0.5f;
        float textX = abs.x + padding - m_scrollOffsetX;

        if (m_text.empty() && !focused) {
            ctx.GetFont().DrawText(ctx.GetRenderer(), m_placeholder,
                                   textX, textY, m_placeholderColor);
        } else {
            // Draw selection highlight
            if (HasSelection()) {
                int selStart = std::min(m_selectionAnchor, m_cursorPos);
                int selEnd = std::max(m_selectionAnchor, m_cursorPos);
                float selStartX = textX + ctx.GetFont().MeasureSubstring(m_text, selStart);
                float selEndX = textX + ctx.GetFont().MeasureSubstring(m_text, selEnd);
                glm::vec4 selColor = m_focusBorderColor;
                selColor.a = 0.3f;
                ctx.GetRenderer().DrawQuad({selStartX, abs.y + 3.0f},
                                           {selEndX - selStartX, m_size.y - 6.0f}, selColor);
            }

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

        SDL_Keymod mod = SDL_GetModState();
        bool shift = (mod & KMOD_SHIFT) != 0;
        bool ctrl = (mod & KMOD_CTRL) != 0;

        switch (e.scancode) {
        case SDL_SCANCODE_BACKSPACE:
            if (HasSelection()) {
                DeleteSelection();
            } else if (m_cursorPos > 0) {
                EraseCharAt(m_cursorPos - 1);
                m_cursorPos--;
            }
            OnTextModified();
            return true;
        case SDL_SCANCODE_DELETE:
            if (HasSelection()) {
                DeleteSelection();
            } else if (m_cursorPos < (int)CountChars(m_text)) {
                EraseCharAt(m_cursorPos);
            }
            OnTextModified();
            return true;
        case SDL_SCANCODE_LEFT:
            if (shift) {
                StartSelectionIfNeeded();
                if (m_cursorPos > 0) m_cursorPos--;
            } else {
                if (HasSelection()) {
                    m_cursorPos = std::min(m_selectionAnchor, m_cursorPos);
                    ClearSelection();
                } else if (m_cursorPos > 0) {
                    m_cursorPos--;
                }
            }
            ResetBlink();
            return true;
        case SDL_SCANCODE_RIGHT:
            if (shift) {
                StartSelectionIfNeeded();
                if (m_cursorPos < (int)CountChars(m_text)) m_cursorPos++;
            } else {
                if (HasSelection()) {
                    m_cursorPos = std::max(m_selectionAnchor, m_cursorPos);
                    ClearSelection();
                } else if (m_cursorPos < (int)CountChars(m_text)) {
                    m_cursorPos++;
                }
            }
            ResetBlink();
            return true;
        case SDL_SCANCODE_HOME:
            if (shift) StartSelectionIfNeeded();
            else ClearSelection();
            m_cursorPos = 0;
            ResetBlink();
            return true;
        case SDL_SCANCODE_END:
            if (shift) StartSelectionIfNeeded();
            else ClearSelection();
            m_cursorPos = (int)CountChars(m_text);
            ResetBlink();
            return true;
        case SDL_SCANCODE_A:
            if (ctrl) {
                m_selectionAnchor = 0;
                m_cursorPos = (int)CountChars(m_text);
                ResetBlink();
                return true;
            }
            return false;
        case SDL_SCANCODE_C:
            if (ctrl && HasSelection()) {
                SDL_SetClipboardText(GetSelectedText().c_str());
                return true;
            }
            return false;
        case SDL_SCANCODE_X:
            if (ctrl && HasSelection()) {
                SDL_SetClipboardText(GetSelectedText().c_str());
                DeleteSelection();
                OnTextModified();
                return true;
            }
            return false;
        case SDL_SCANCODE_V:
            if (ctrl) {
                char* clipboard = SDL_GetClipboardText();
                if (clipboard) {
                    DeleteSelection();
                    InsertText(clipboard);
                    SDL_free(clipboard);
                }
                return true;
            }
            return false;
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

    // --- Selection ---

    bool HasSelection() const {
        return m_selectionAnchor >= 0 && m_selectionAnchor != m_cursorPos;
    }

    void StartSelectionIfNeeded() {
        if (m_selectionAnchor < 0)
            m_selectionAnchor = m_cursorPos;
    }

    void ClearSelection() { m_selectionAnchor = -1; }

    std::string GetSelectedText() const {
        if (!HasSelection()) return "";
        int start = std::min(m_selectionAnchor, m_cursorPos);
        int end = std::max(m_selectionAnchor, m_cursorPos);
        int byteStart = CharIndexToByteIndex(m_text, start);
        int byteEnd = CharIndexToByteIndex(m_text, end);
        return m_text.substr(byteStart, byteEnd - byteStart);
    }

    void DeleteSelection() {
        if (!HasSelection()) return;
        int start = std::min(m_selectionAnchor, m_cursorPos);
        int end = std::max(m_selectionAnchor, m_cursorPos);
        int byteStart = CharIndexToByteIndex(m_text, start);
        int byteEnd = CharIndexToByteIndex(m_text, end);
        m_text.erase(byteStart, byteEnd - byteStart);
        m_cursorPos = start;
        ClearSelection();
    }

    // --- Text editing ---

    void InsertText(const char* text) {
        std::string ins(text);
        int bytePos = CharIndexToByteIndex(m_text, m_cursorPos);
        m_text.insert(bytePos, ins);
        m_cursorPos += (int)CountChars(ins);
        OnTextModified();
    }

    void OnTextModified() {
        ResetBlink();
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

    void EnsureCursorVisible(UIContext& ctx, float textAreaW) {
        float cursorPixelPos = ctx.GetFont().MeasureSubstring(m_text, m_cursorPos);
        if (cursorPixelPos - m_scrollOffsetX > textAreaW)
            m_scrollOffsetX = cursorPixelPos - textAreaW;
        if (cursorPixelPos - m_scrollOffsetX < 0)
            m_scrollOffsetX = cursorPixelPos;
        if (m_scrollOffsetX < 0)
            m_scrollOffsetX = 0;
    }

    void PositionCursorFromClick(UIContext& ctx) {
        glm::vec2 abs = GetAbsolutePos();
        float padding = 6.0f;
        float clickX = ctx.GetMousePos().x - abs.x - padding + m_scrollOffsetX;

        int charCount = (int)CountChars(m_text);
        int best = 0;
        float bestDist = clickX;

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
    std::string m_placeholderI18nKey;
    bool m_langSubscribed = false;
    int m_cursorPos = 0;
    int m_selectionAnchor = -1;  // -1 = no selection
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

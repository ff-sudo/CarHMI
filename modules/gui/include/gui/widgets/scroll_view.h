#pragma once

#include <gui/widget.h>
#include <core/event_bus.h>
#include <core/event.h>
#include <algorithm>

namespace CarHMI::GUI {

class ScrollView : public Widget {
public:
    ScrollView(int id, glm::vec2 pos, glm::vec2 size,
               glm::vec4 bgColor = {0.12f, 0.12f, 0.16f, 1.0f})
        : Widget(id, pos, size), m_bgColor(bgColor) {
        m_connections.Add(Core::EventBus::Get().Subscribe<Core::MouseScrollEvent>(
            [this](const Core::MouseScrollEvent& e) {
                OnMouseScroll(e);
            }));
        m_connections.Add(Core::EventBus::Get().Subscribe<Core::MouseButtonEvent>(
            [this](const Core::MouseButtonEvent& e) {
                OnMouseButton(e);
            }));
        m_connections.Add(Core::EventBus::Get().Subscribe<Core::MouseMoveEvent>(
            [this](const Core::MouseMoveEvent& e) {
                OnMouseMove(e);
            }));
    }

    void AddChild(Widget* child) {
        Widget::AddChild(child);
        RecalcContentSize();
    }

    // Content size
    void SetContentSize(glm::vec2 s) { m_contentSize = s; }
    glm::vec2 GetContentSize() const { return m_contentSize; }

    // Scroll offset (vertical)
    float GetScrollOffsetY() const { return m_scrollY; }
    void SetScrollOffsetY(float y) { m_scrollY = y; ClampScroll(); }
    void ScrollByY(float delta) { m_scrollY += delta; ClampScroll(); }

    // Scroll offset (horizontal)
    float GetScrollOffsetX() const { return m_scrollX; }
    void SetScrollOffsetX(float x) { m_scrollX = x; ClampScroll(); }
    void ScrollByX(float delta) { m_scrollX += delta; ClampScroll(); }

    // Legacy API compatibility
    float GetScrollOffset() const { return m_scrollY; }
    void SetScrollOffset(float offset) { m_scrollY = offset; ClampScroll(); }
    void ScrollBy(float delta) { m_scrollY += delta; ClampScroll(); }
    float GetContentHeight() const { return m_contentSize.y; }
    void SetContentHeight(float h) { m_contentSize.y = h; }

    void SetScrollSpeed(float pixelsPerTick) { m_scrollSpeed = pixelsPerTick; }
    void SetShowScrollbar(bool show) { m_showScrollbar = show; }
    void SetBackgroundColor(glm::vec4 c) { m_bgColor = c; }

    void Update(UIContext& ctx) override {
        if (!m_visible) return;
        m_lastMousePos = ctx.GetMousePos();

        for (auto* child : m_children) {
            glm::vec2 origPos = child->GetPos();
            child->SetPos(origPos + glm::vec2(-m_scrollX, -m_scrollY));
            child->Update(ctx);
            child->SetPos(origPos);
        }
    }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();

        // Background
        ctx.GetRenderer().DrawQuad(abs, m_size, m_bgColor);

        // Reserve space for scrollbars
        float clipW = m_size.x - (NeedsVScroll() ? kBarWidth + kBarMargin * 2 : 0);
        float clipH = m_size.y - (NeedsHScroll() ? kBarWidth + kBarMargin * 2 : 0);

        // Clip children to content area (excluding scrollbar region)
        ctx.PushClipRect(abs, {clipW, clipH});
        for (auto* child : m_children) {
            glm::vec2 origPos = child->GetPos();
            child->SetPos(origPos + glm::vec2(-m_scrollX, -m_scrollY));
            child->Draw(ctx);
            child->SetPos(origPos);
        }
        ctx.PopClipRect();

        // Draw scrollbars on top
        if (m_showScrollbar) {
            if (NeedsVScroll())
                DrawVScrollbar(ctx, abs);
            if (NeedsHScroll())
                DrawHScrollbar(ctx, abs);
        }
    }

    void OnFocusAdjust(float delta) override {
        ScrollByY(-delta * m_scrollSpeed);
    }

private:
    // --- Mouse event handlers ---

    void OnMouseScroll(const Core::MouseScrollEvent& e) {
        if (!IsHovering()) return;
        ScrollByY(-e.yOffset * m_scrollSpeed);
        if (e.xOffset != 0.0f)
            ScrollByX(-e.xOffset * m_scrollSpeed);
    }

    void OnMouseButton(const Core::MouseButtonEvent& e) {
        if (e.button != Core::MouseButton::Left) return;

        if (e.pressed) {
            // Check if clicking on vertical scrollbar thumb
            if (NeedsVScroll()) {
                auto thumbRect = GetVThumbRect();
                if (HitTest(m_lastMousePos, thumbRect)) {
                    m_draggingV = true;
                    m_dragStartMouseY = m_lastMousePos.y;
                    m_dragStartScroll = m_scrollY;
                    return;
                }
            }
            // Check horizontal scrollbar thumb
            if (NeedsHScroll()) {
                auto thumbRect = GetHThumbRect();
                if (HitTest(m_lastMousePos, thumbRect)) {
                    m_draggingH = true;
                    m_dragStartMouseX = m_lastMousePos.x;
                    m_dragStartScroll = m_scrollX;
                    return;
                }
            }
        } else {
            m_draggingV = false;
            m_draggingH = false;
        }
    }

    void OnMouseMove(const Core::MouseMoveEvent& e) {
        if (m_draggingV) {
            glm::vec2 abs = GetAbsolutePos();
            float trackH = m_size.y - kBarMargin * 2 - (NeedsHScroll() ? kBarWidth + 2 : 0);
            float thumbH = GetVThumbHeight(trackH);
            float movableRange = trackH - thumbH;
            if (movableRange > 0) {
                float mouseDelta = (float)e.y - m_dragStartMouseY;
                float scrollDelta = (mouseDelta / movableRange) * MaxScrollY();
                m_scrollY = m_dragStartScroll + scrollDelta;
                ClampScroll();
            }
        }
        if (m_draggingH) {
            glm::vec2 abs = GetAbsolutePos();
            float trackW = m_size.x - kBarMargin * 2 - (NeedsVScroll() ? kBarWidth + 2 : 0);
            float thumbW = GetHThumbWidth(trackW);
            float movableRange = trackW - thumbW;
            if (movableRange > 0) {
                float mouseDelta = (float)e.x - m_dragStartMouseX;
                float scrollDelta = (mouseDelta / movableRange) * MaxScrollX();
                m_scrollX = m_dragStartScroll + scrollDelta;
                ClampScroll();
            }
        }
    }

    // --- Helpers ---

    bool IsHovering() const {
        glm::vec2 abs = GetAbsolutePos();
        return m_lastMousePos.x >= abs.x && m_lastMousePos.x <= abs.x + m_size.x &&
               m_lastMousePos.y >= abs.y && m_lastMousePos.y <= abs.y + m_size.y;
    }

    bool NeedsVScroll() const { return m_contentSize.y > m_size.y; }
    bool NeedsHScroll() const { return m_contentSize.x > m_size.x; }

    float MaxScrollY() const { return std::max(0.0f, m_contentSize.y - m_size.y); }
    float MaxScrollX() const { return std::max(0.0f, m_contentSize.x - m_size.x); }

    void ClampScroll() {
        m_scrollY = std::clamp(m_scrollY, 0.0f, MaxScrollY());
        m_scrollX = std::clamp(m_scrollX, 0.0f, MaxScrollX());
    }

    void RecalcContentSize() {
        float maxRight = 0, maxBottom = 0;
        for (auto* child : m_children) {
            float right = child->GetPos().x + child->GetSize().x + child->GetMargin().right;
            float bottom = child->GetPos().y + child->GetSize().y + child->GetMargin().bottom;
            maxRight = std::max(maxRight, right);
            maxBottom = std::max(maxBottom, bottom);
        }
        m_contentSize = {maxRight, maxBottom};
    }

    // --- Scrollbar geometry ---

    static constexpr float kBarWidth = 6.0f;
    static constexpr float kBarMargin = 2.0f;
    static constexpr float kMinThumb = 20.0f;

    float GetVThumbHeight(float trackH) const {
        float ratio = m_size.y / m_contentSize.y;
        return std::max(kMinThumb, trackH * ratio);
    }

    float GetHThumbWidth(float trackW) const {
        float ratio = m_size.x / m_contentSize.x;
        return std::max(kMinThumb, trackW * ratio);
    }

    struct Rect { float x, y, w, h; };

    Rect GetVThumbRect() const {
        glm::vec2 abs = GetAbsolutePos();
        float trackH = m_size.y - kBarMargin * 2 - (NeedsHScroll() ? kBarWidth + 2 : 0);
        float thumbH = GetVThumbHeight(trackH);
        float scrollRatio = (MaxScrollY() > 0) ? m_scrollY / MaxScrollY() : 0;
        float thumbY = abs.y + kBarMargin + scrollRatio * (trackH - thumbH);
        float thumbX = abs.x + m_size.x - kBarWidth - kBarMargin;
        return {thumbX, thumbY, kBarWidth, thumbH};
    }

    Rect GetHThumbRect() const {
        glm::vec2 abs = GetAbsolutePos();
        float trackW = m_size.x - kBarMargin * 2 - (NeedsVScroll() ? kBarWidth + 2 : 0);
        float thumbW = GetHThumbWidth(trackW);
        float scrollRatio = (MaxScrollX() > 0) ? m_scrollX / MaxScrollX() : 0;
        float thumbX = abs.x + kBarMargin + scrollRatio * (trackW - thumbW);
        float thumbY = abs.y + m_size.y - kBarWidth - kBarMargin;
        return {thumbX, thumbY, thumbW, kBarWidth};
    }

    static bool HitTest(glm::vec2 point, Rect r) {
        return point.x >= r.x && point.x <= r.x + r.w &&
               point.y >= r.y && point.y <= r.y + r.h;
    }

    // --- Scrollbar drawing ---

    void DrawVScrollbar(UIContext& ctx, glm::vec2 abs) {
        float trackX = abs.x + m_size.x - kBarWidth - kBarMargin;
        float trackY = abs.y + kBarMargin;
        float trackH = m_size.y - kBarMargin * 2 - (NeedsHScroll() ? kBarWidth + 2 : 0);

        // Track
        ctx.GetRenderer().DrawRoundedRect({trackX, trackY}, {kBarWidth, trackH}, 3.0f,
                                          {0.25f, 0.25f, 0.3f, 0.4f});
        // Thumb
        auto thumb = GetVThumbRect();
        glm::vec4 thumbColor = m_draggingV
            ? glm::vec4(0.9f, 0.9f, 0.95f, 0.9f)
            : glm::vec4(0.6f, 0.6f, 0.65f, 0.8f);
        ctx.GetRenderer().DrawRoundedRect({thumb.x, thumb.y}, {thumb.w, thumb.h}, 3.0f, thumbColor);
    }

    void DrawHScrollbar(UIContext& ctx, glm::vec2 abs) {
        float trackX = abs.x + kBarMargin;
        float trackY = abs.y + m_size.y - kBarWidth - kBarMargin;
        float trackW = m_size.x - kBarMargin * 2 - (NeedsVScroll() ? kBarWidth + 2 : 0);

        // Track
        ctx.GetRenderer().DrawRoundedRect({trackX, trackY}, {trackW, kBarWidth}, 3.0f,
                                          {0.25f, 0.25f, 0.3f, 0.4f});
        // Thumb
        auto thumb = GetHThumbRect();
        glm::vec4 thumbColor = m_draggingH
            ? glm::vec4(0.9f, 0.9f, 0.95f, 0.9f)
            : glm::vec4(0.6f, 0.6f, 0.65f, 0.8f);
        ctx.GetRenderer().DrawRoundedRect({thumb.x, thumb.y}, {thumb.w, thumb.h}, 3.0f, thumbColor);
    }

    // --- State ---
    float m_scrollX = 0;
    float m_scrollY = 0;
    glm::vec2 m_contentSize = {0, 0};
    float m_scrollSpeed = 30.0f;
    bool m_showScrollbar = true;
    glm::vec4 m_bgColor;
    glm::vec2 m_lastMousePos = {0, 0};

    // Scrollbar drag state
    bool m_draggingV = false;
    bool m_draggingH = false;
    float m_dragStartMouseY = 0;
    float m_dragStartMouseX = 0;
    float m_dragStartScroll = 0;

    Core::ConnectionGroup m_connections;
};

} // namespace CarHMI::GUI

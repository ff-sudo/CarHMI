#include "ui_context.h"
#include "core/event_bus.h"
#include "core/event.h"
#include <spdlog/spdlog.h>

namespace CarHMI {

void UIContext::Init(BatchRenderer2D* renderer, Font* font) {
    m_renderer = renderer;
    m_font = font;

    EventBus::Get().subscribe<FrameBeginEvent>([this](const FrameBeginEvent&) {
        m_mousePressed = false;
        m_mouseReleased = false;
        m_hotId = 0;
    });

    EventBus::Get().subscribe<MouseMoveEvent>([this](const MouseMoveEvent& e) {
        m_mousePos = {e.x, e.y};
    });

    EventBus::Get().subscribe<MouseButtonEvent>([this](const MouseButtonEvent& e) {
        if (e.button == MouseButton::Left) {
            m_mouseDown = e.pressed;
            if (e.pressed) {
                m_mousePressed = true;
                spdlog::debug("Mouse pressed at ({:.0f}, {:.0f})", e.x, e.y);
            } else {
                m_mouseReleased = true;
                spdlog::debug("Mouse released at ({:.0f}, {:.0f})", e.x, e.y);
            }
        }
    });

    spdlog::info("UIContext initialized (event bus)");
}

void UIContext::BeginFrame() {
    m_mousePressed = false;
    m_mouseReleased = false;
    m_hotId = 0;
}

} // namespace CarHMI

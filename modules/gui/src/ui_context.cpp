#include <gui/ui_context.h>
#include <core/event_bus.h>
#include <core/event.h>
#include <spdlog/spdlog.h>

namespace CarHMI::GUI {

void UIContext::Init(RHI::BatchRenderer2D* renderer, RHI::Font* font) {
    m_renderer = renderer;
    m_font = font;

    Core::EventBus::Get().subscribe<Core::FrameBeginEvent>([this](const Core::FrameBeginEvent&) {
        m_mousePressed = false;
        m_mouseReleased = false;
        m_hotId = 0;
    });

    Core::EventBus::Get().subscribe<Core::MouseMoveEvent>([this](const Core::MouseMoveEvent& e) {
        m_mousePos = {e.x, e.y};
    });

    Core::EventBus::Get().subscribe<Core::MouseButtonEvent>([this](const Core::MouseButtonEvent& e) {
        if (e.button == Core::MouseButton::Left) {
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

} // namespace CarHMI::GUI

#include <gui/ui_context.h>
#include <core/event_bus.h>
#include <core/event.h>
#include <spdlog/spdlog.h>

namespace CarHMI::GUI {

void UIContext::Init(RHI::BatchRenderer2D* renderer, RHI::Font* font) {
    m_renderer = renderer;
    m_font = font;

    m_connections.Add(Core::EventBus::Get().Subscribe<Core::FrameBeginEvent>(
        [this](const Core::FrameBeginEvent&) {
            m_mousePressed = false;
            m_mouseReleased = false;
            m_hotId = 0;
        }));

    m_connections.Add(Core::EventBus::Get().Subscribe<Core::MouseMoveEvent>(
        [this](const Core::MouseMoveEvent& e) {
            m_mousePos = {e.x, e.y};
        }));

    m_connections.Add(Core::EventBus::Get().Subscribe<Core::MouseButtonEvent>(
        [this](const Core::MouseButtonEvent& e) {
            if (e.button == Core::MouseButton::Left) {
                m_mouseDown = e.pressed;
                if (e.pressed) {
                    m_mousePressed = true;
                } else {
                    m_mouseReleased = true;
                }
            }
        }));

    spdlog::info("UIContext initialized (event bus)");
}

void UIContext::BeginFrame() {
    m_mousePressed = false;
    m_mouseReleased = false;
    m_hotId = 0;
}

void UIContext::PushClipRect(glm::vec2 pos, glm::vec2 size) {
    m_renderer->PushScissor(pos, size);
}

void UIContext::PopClipRect() {
    m_renderer->PopScissor();
}

} // namespace CarHMI::GUI

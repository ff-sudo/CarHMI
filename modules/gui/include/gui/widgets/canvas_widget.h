#pragma once

#include <gui/widget.h>
#include <gui/canvas.h>
#include <functional>

namespace CarHMI::GUI {

class CanvasWidget : public Widget {
public:
    using DrawCallback = std::function<void(Canvas& cv)>;

    CanvasWidget(int id, glm::vec2 pos, glm::vec2 size)
        : Widget(id, pos, size) {}

    void SetOnDraw(DrawCallback cb) { m_onDraw = std::move(cb); }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        Canvas cv(ctx.GetRenderer(), ctx.GetFont(), GetAbsolutePos());
        if (m_onDraw) m_onDraw(cv);
        Widget::Draw(ctx);
    }

private:
    DrawCallback m_onDraw;
};

} // namespace CarHMI::GUI

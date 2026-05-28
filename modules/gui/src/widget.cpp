#include <gui/widget.h>
#include <gui/focus_manager.h>
#include <gui/style/theme_manager.h>
#include <algorithm>

namespace CarHMI::GUI {

Widget::~Widget() {
    m_themeSubscription.Disconnect();
    if (m_focusable)
        FocusManager::Get().UnregisterWidget(this);
    for (auto* child : m_children)
        delete child;
}

void Widget::Draw(UIContext& ctx) {
    if (!m_visible) return;
    for (auto* child : m_children)
        child->Draw(ctx);
}

void Widget::Update(UIContext& ctx) {
    if (!m_visible) return;
    for (auto* child : m_children)
        child->Update(ctx);
}

void Widget::DrawFocusHighlight(UIContext& ctx) {
    if (!FocusManager::Get().IsFocused(m_id)) return;

    glm::vec2 abs = GetAbsolutePos();
    glm::vec4 highlightColor = ThemeManager::Get().GetTheme().widget.accentColor;
    highlightColor.a = 0.7f;
    float t = 2.0f;

    ctx.GetRenderer().DrawQuad({abs.x - t, abs.y - t}, {m_size.x + 2*t, t}, highlightColor);
    ctx.GetRenderer().DrawQuad({abs.x - t, abs.y + m_size.y}, {m_size.x + 2*t, t}, highlightColor);
    ctx.GetRenderer().DrawQuad({abs.x - t, abs.y}, {t, m_size.y}, highlightColor);
    ctx.GetRenderer().DrawQuad({abs.x + m_size.x, abs.y}, {t, m_size.y}, highlightColor);
}

void Widget::AddChild(Widget* child) {
    child->m_parent = this;
    m_children.push_back(child);
}

void Widget::RemoveChild(Widget* child) {
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        (*it)->m_parent = nullptr;
        m_children.erase(it);
    }
}

bool Widget::Contains(glm::vec2 point) const {
    glm::vec2 abs = GetAbsolutePos();
    return point.x >= abs.x && point.x <= abs.x + m_size.x &&
           point.y >= abs.y && point.y <= abs.y + m_size.y;
}

glm::vec2 Widget::GetAbsolutePos() const {
    if (m_parent)
        return m_parent->GetAbsolutePos() + m_pos + m_parent->m_scrollOffset;
    return m_pos;
}

void Widget::SubscribeThemeChange() {
    m_themeSubscription = Core::EventBus::Get().Subscribe<Core::ThemeChangedEvent>(
        [this](const Core::ThemeChangedEvent&) {
            ApplyThemeRecursive();
        }
    );
}

void Widget::ApplyThemeRecursive() {
    ApplyTheme();
    for (auto* child : m_children)
        child->ApplyThemeRecursive();
}

void Widget::RegisterFocusRecursive() {
    if (m_focusable)
        FocusManager::Get().RegisterWidget(this);
    for (auto* child : m_children)
        child->RegisterFocusRecursive();
}

void Widget::UnregisterFocusRecursive() {
    if (m_focusable)
        FocusManager::Get().UnregisterWidget(this);
    for (auto* child : m_children)
        child->UnregisterFocusRecursive();
}

} // namespace CarHMI::GUI

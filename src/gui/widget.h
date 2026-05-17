#pragma once

#include "ui_context.h"
#include "core/event_bus.h"
#include "core/event.h"
#include "core/property/property.h"
#include "core/property/property_map.h"
#include <glm/glm.hpp>
#include <vector>

namespace CarHMI {

class Widget {
public:
    Widget(int id, glm::vec2 pos, glm::vec2 size)
        : m_id(id), m_pos(pos), m_size(size) {}
    virtual ~Widget();

    virtual void Draw(UIContext& ctx);
    virtual void Update(UIContext& ctx);
    virtual void ApplyTheme() {}

    virtual void OnFocusActivate() {}
    virtual void OnFocusAdjust(float delta) {}

    void DrawFocusHighlight(UIContext& ctx);

    void AddChild(Widget* child);
    bool Contains(glm::vec2 point) const;

    void SubscribeThemeChange();
    void ApplyThemeRecursive();

    void SetFocusable(bool f) { m_focusable = f; }
    bool IsFocusable() const { return m_focusable; }
    void RegisterFocusRecursive();
    void UnregisterFocusRecursive();

    void SetPos(glm::vec2 pos) { m_pos = pos; }
    void SetSize(glm::vec2 size) { m_size = size; }
    void SetVisible(bool v) { m_visible = v; }

    glm::vec2 GetPos() const { return m_pos; }
    glm::vec2 GetSize() const { return m_size; }
    glm::vec2 GetAbsolutePos() const;
    int GetID() const { return m_id; }
    bool IsVisible() const { return m_visible; }

    const std::vector<Widget*>& GetChildren() const { return m_children; }
    PropertyMap& Props() { return m_props; }

protected:
    int m_id;
    glm::vec2 m_pos;
    glm::vec2 m_size;
    bool m_visible = true;
    bool m_focusable = false;
    Widget* m_parent = nullptr;
    std::vector<Widget*> m_children;
    SubscriptionHandle m_themeSubscription;
    PropertyMap m_props;
};

} // namespace CarHMI

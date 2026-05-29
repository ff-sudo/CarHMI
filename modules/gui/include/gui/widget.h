#pragma once

#include "ui_context.h"
#include <gui/layout/layout_data.h>
#include <core/event_bus.h>
#include <core/event.h>
#include <core/property/property.h>
#include <core/property/property_map.h>
#include <core/animation/animation_manager.h>
#include <core/animation/easing.h>
#include <glm/glm.hpp>
#include <vector>

namespace CarHMI::GUI {

using Core::Property;
using Core::PropertyMap;
using Core::Connection;
using Core::ConnectionGroup;
using Core::AnimationManager;
using Core::EasingFunc;
namespace Easing = Core::Easing;

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
    virtual bool OnKeyEvent(const Core::KeyEvent&) { return false; }

    void DrawFocusHighlight(UIContext& ctx);

    virtual void AddChild(Widget* child);
    virtual void RemoveChild(Widget* child);
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

    void SetMargin(Insets m) { GetLayoutData().margin = m; }
    void SetMargin(float all) { GetLayoutData().margin = Insets(all); }
    Insets GetMargin() const { return GetLayoutData().margin; }

    void SetSizePolicy(SizePolicy sp) { GetLayoutData().sizePolicy = sp; }
    SizePolicy GetSizePolicy() const { return GetLayoutData().sizePolicy; }

    void SetWidthPercent(float pct) {
        auto& ld = GetLayoutData();
        ld.sizePolicy.widthMode = SizeMode::Percent;
        ld.sizePolicy.widthValue = pct;
    }
    void SetHeightPercent(float pct) {
        auto& ld = GetLayoutData();
        ld.sizePolicy.heightMode = SizeMode::Percent;
        ld.sizePolicy.heightValue = pct;
    }
    void SetFillWidth() { GetLayoutData().sizePolicy.widthMode = SizeMode::Fill; }
    void SetFillHeight() { GetLayoutData().sizePolicy.heightMode = SizeMode::Fill; }

    glm::vec2 GetPos() const { return m_pos; }
    glm::vec2 GetSize() const { return m_size; }
    glm::vec2 GetAbsolutePos() const;
    int GetID() const { return m_id; }
    bool IsVisible() const { return m_visible; }

    const std::vector<Widget*>& GetChildren() const { return m_children; }
    Core::PropertyMap& Props() { return m_props; }

protected:
    int m_id;
    glm::vec2 m_pos;
    glm::vec2 m_size;
    bool m_visible = true;
    bool m_focusable = false;
    Widget* m_parent = nullptr;
    std::unique_ptr<LayoutData> m_layoutData;  // lazy-allocated
    std::vector<Widget*> m_children;
    Core::Connection m_themeSubscription;
    Core::PropertyMap m_props;

    // Tree mutation safety: mutations during Update/Draw are deferred
    int m_updateDepth = 0;
    std::vector<Widget*> m_pendingAdditions;
    std::vector<Widget*> m_pendingRemovals;

    void DoRemoveChild(Widget* child);
    void FlushPendingMutations();

    LayoutData& GetLayoutData() {
        if (!m_layoutData) m_layoutData = std::make_unique<LayoutData>();
        return *m_layoutData;
    }
    const LayoutData& GetLayoutData() const {
        return const_cast<Widget*>(this)->GetLayoutData();
    }
};

} // namespace CarHMI::GUI

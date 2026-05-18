#pragma once

#include <core/event_bus.h>
#include <core/event.h>
#include <vector>
#include <spdlog/spdlog.h>
#include <SDL.h>

namespace CarHMI::GUI {

class Widget;

class FocusManager {
public:
    static FocusManager& Get() {
        static FocusManager instance;
        return instance;
    }

    void Init();

    void RegisterWidget(Widget* w);
    void UnregisterWidget(Widget* w);
    void ClearAll();

    void FocusNext();
    void FocusPrev();
    void ActivateFocused();
    void AdjustFocusedValue(float delta);

    Widget* GetFocused() const;
    int GetFocusedId() const;
    bool IsFocused(int widgetId) const;

    void SetFocusIndex(int idx);

private:
    FocusManager() = default;

    std::vector<Widget*> m_focusChain;
    int m_focusIndex = -1;
    bool m_initialized = false;

    Core::SubscriptionHandle m_keySub;
    Core::SubscriptionHandle m_scrollSub;
};

} // namespace CarHMI::GUI

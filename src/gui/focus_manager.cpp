#include "focus_manager.h"
#include "widget.h"
#include <algorithm>

namespace CarHMI {

void FocusManager::Init() {
    if (m_initialized) return;
    m_initialized = true;

    m_keySub = EventBus::Get().subscribe<KeyEvent>([this](const KeyEvent& e) {
        if (!e.pressed) return;

        switch (e.scancode) {
        case SDL_SCANCODE_TAB:
            FocusNext();
            break;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_LEFT:
            FocusPrev();
            break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_RIGHT:
            FocusNext();
            break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_SPACE:
            ActivateFocused();
            break;
        }
    });

    m_scrollSub = EventBus::Get().subscribe<MouseScrollEvent>([this](const MouseScrollEvent& e) {
        if (m_focusIndex >= 0)
            AdjustFocusedValue(e.yOffset);
    });

    spdlog::info("FocusManager initialized");
}

void FocusManager::RegisterWidget(Widget* w) {
    m_focusChain.push_back(w);
    if (m_focusIndex < 0)
        m_focusIndex = 0;
}

void FocusManager::UnregisterWidget(Widget* w) {
    auto it = std::find(m_focusChain.begin(), m_focusChain.end(), w);
    if (it != m_focusChain.end()) {
        int idx = (int)(it - m_focusChain.begin());
        m_focusChain.erase(it);
        if (m_focusChain.empty())
            m_focusIndex = -1;
        else if (m_focusIndex >= (int)m_focusChain.size())
            m_focusIndex = (int)m_focusChain.size() - 1;
    }
}

void FocusManager::ClearAll() {
    m_focusChain.clear();
    m_focusIndex = -1;
}

void FocusManager::FocusNext() {
    if (m_focusChain.empty()) return;
    m_focusIndex = (m_focusIndex + 1) % (int)m_focusChain.size();
    spdlog::debug("Focus -> widget[{}] id={}", m_focusIndex, m_focusChain[m_focusIndex]->GetID());
}

void FocusManager::FocusPrev() {
    if (m_focusChain.empty()) return;
    m_focusIndex--;
    if (m_focusIndex < 0) m_focusIndex = (int)m_focusChain.size() - 1;
    spdlog::debug("Focus -> widget[{}] id={}", m_focusIndex, m_focusChain[m_focusIndex]->GetID());
}

void FocusManager::ActivateFocused() {
    Widget* w = GetFocused();
    if (w) {
        spdlog::info("Focus activate widget id={}", w->GetID());
        w->OnFocusActivate();
    }
}

void FocusManager::AdjustFocusedValue(float delta) {
    Widget* w = GetFocused();
    if (w) w->OnFocusAdjust(delta);
}

Widget* FocusManager::GetFocused() const {
    if (m_focusIndex < 0 || m_focusIndex >= (int)m_focusChain.size())
        return nullptr;
    return m_focusChain[m_focusIndex];
}

int FocusManager::GetFocusedId() const {
    Widget* w = GetFocused();
    return w ? w->GetID() : -1;
}

bool FocusManager::IsFocused(int widgetId) const {
    Widget* w = GetFocused();
    return w && w->GetID() == widgetId;
}

void FocusManager::SetFocusIndex(int idx) {
    if (idx >= 0 && idx < (int)m_focusChain.size())
        m_focusIndex = idx;
}

} // namespace CarHMI

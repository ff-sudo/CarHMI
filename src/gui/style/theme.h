#pragma once

#include <glm/glm.hpp>
#include <string>

namespace CarHMI {

struct WidgetStyle {
    // Colors
    glm::vec4 backgroundColor = {0.15f, 0.15f, 0.18f, 0.95f};
    glm::vec4 foregroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 accentColor     = {0.2f, 0.6f, 0.9f, 1.0f};

    // Spacing
    float padding = 10.0f;
    float spacing = 8.0f;
    float borderRadius = 0.0f;
};

struct ButtonStyle {
    glm::vec4 normalColor  = {0.25f, 0.25f, 0.30f, 1.0f};
    glm::vec4 hoverColor   = {0.35f, 0.35f, 0.42f, 1.0f};
    glm::vec4 pressColor   = {0.18f, 0.18f, 0.22f, 1.0f};
    glm::vec4 textColor    = {1.0f, 1.0f, 1.0f, 1.0f};
    float transitionTime   = 0.15f;
};

struct SliderStyle {
    glm::vec4 trackColor      = {0.2f, 0.2f, 0.25f, 1.0f};
    glm::vec4 fillColor       = {0.2f, 0.6f, 0.9f, 1.0f};
    glm::vec4 handleColor     = {0.8f, 0.8f, 0.85f, 1.0f};
    glm::vec4 handleActive    = {1.0f, 1.0f, 1.0f, 1.0f};
    float trackHeight         = 6.0f;
    float handleWidth         = 14.0f;
    float handleHeight        = 22.0f;
};

struct ProgressBarStyle {
    glm::vec4 backgroundColor = {0.2f, 0.2f, 0.25f, 1.0f};
    glm::vec4 fillColor       = {0.2f, 0.8f, 0.4f, 1.0f};
    glm::vec4 warningColor    = {0.9f, 0.7f, 0.1f, 1.0f};
    glm::vec4 dangerColor     = {0.9f, 0.2f, 0.2f, 1.0f};
};

struct LabelStyle {
    glm::vec4 textColor = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 titleColor = {0.4f, 0.8f, 1.0f, 1.0f};
    glm::vec4 subtitleColor = {0.7f, 0.9f, 1.0f, 1.0f};
};

struct PanelStyle {
    glm::vec4 backgroundColor = {0.15f, 0.15f, 0.18f, 0.95f};
};

struct ToggleStyle {
    glm::vec4 offColor    = {0.3f, 0.3f, 0.35f, 1.0f};
    glm::vec4 onColor     = {0.2f, 0.7f, 0.4f, 1.0f};
    glm::vec4 handleColor = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct KnobStyle {
    glm::vec4 bgColor        = {0.18f, 0.18f, 0.22f, 1.0f};
    glm::vec4 knobColor      = {0.28f, 0.28f, 0.32f, 1.0f};
    glm::vec4 accentColor    = {0.2f, 0.6f, 0.9f, 1.0f};
    glm::vec4 indicatorColor = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct GaugeStyle {
    glm::vec4 bgColor      = {0.12f, 0.12f, 0.15f, 1.0f};
    glm::vec4 trackColor   = {0.25f, 0.25f, 0.30f, 1.0f};
    glm::vec4 fillColor    = {0.2f, 0.6f, 0.9f, 1.0f};
    glm::vec4 warningColor = {0.9f, 0.7f, 0.1f, 1.0f};
    glm::vec4 dangerColor  = {0.9f, 0.2f, 0.2f, 1.0f};
    glm::vec4 needleColor  = {1.0f, 0.3f, 0.2f, 1.0f};
    glm::vec4 textColor    = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct TabBarStyle {
    glm::vec4 bgColor           = {0.12f, 0.12f, 0.15f, 1.0f};
    glm::vec4 indicatorColor    = {0.2f, 0.6f, 0.9f, 1.0f};
    glm::vec4 activeTextColor   = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 inactiveTextColor = {0.5f, 0.5f, 0.55f, 1.0f};
};

struct ListStyle {
    glm::vec4 bgColor       = {0.12f, 0.12f, 0.15f, 1.0f};
    glm::vec4 altRowColor   = {0.14f, 0.14f, 0.17f, 0.5f};
    glm::vec4 selectedColor = {0.2f, 0.5f, 0.8f, 0.6f};
    glm::vec4 textColor     = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 scrollbarColor = {0.4f, 0.4f, 0.45f, 0.7f};
};

struct AppStyle {
    glm::vec4 clearColor = {0.08f, 0.08f, 0.10f, 1.0f};
};

struct Theme {
    std::string name;
    AppStyle app;
    PanelStyle panel;
    LabelStyle label;
    ButtonStyle button;
    SliderStyle slider;
    ProgressBarStyle progressBar;
    ToggleStyle toggle;
    KnobStyle knob;
    GaugeStyle gauge;
    TabBarStyle tabBar;
    ListStyle list;
};

} // namespace CarHMI

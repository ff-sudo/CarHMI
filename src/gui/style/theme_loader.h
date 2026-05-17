#pragma once

#include "theme.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

namespace CarHMI {

namespace ThemeLoader {

inline glm::vec4 ParseColor(const nlohmann::json& j, const std::string& key, glm::vec4 fallback) {
    if (!j.contains(key)) return fallback;
    auto& arr = j[key];
    if (!arr.is_array() || arr.size() < 4) return fallback;
    return {arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>(), arr[3].get<float>()};
}

inline float ParseFloat(const nlohmann::json& j, const std::string& key, float fallback) {
    if (!j.contains(key)) return fallback;
    return j[key].get<float>();
}

inline Theme LoadFromFile(const std::string& path) {
    Theme theme;

    std::ifstream f(path);
    if (!f.is_open()) {
        spdlog::warn("Theme: cannot open {}", path);
        return theme;
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(f);
    } catch (const std::exception& e) {
        spdlog::error("Theme: JSON parse error in {}: {}", path, e.what());
        return theme;
    }

    theme.name = root.value("name", "unnamed");

    if (root.contains("app")) {
        auto& j = root["app"];
        theme.app.clearColor = ParseColor(j, "clearColor", theme.app.clearColor);
    }

    if (root.contains("panel")) {
        auto& j = root["panel"];
        theme.panel.backgroundColor = ParseColor(j, "backgroundColor", theme.panel.backgroundColor);
    }

    if (root.contains("label")) {
        auto& j = root["label"];
        theme.label.textColor = ParseColor(j, "textColor", theme.label.textColor);
        theme.label.titleColor = ParseColor(j, "titleColor", theme.label.titleColor);
        theme.label.subtitleColor = ParseColor(j, "subtitleColor", theme.label.subtitleColor);
    }

    if (root.contains("button")) {
        auto& j = root["button"];
        theme.button.normalColor = ParseColor(j, "normalColor", theme.button.normalColor);
        theme.button.hoverColor = ParseColor(j, "hoverColor", theme.button.hoverColor);
        theme.button.pressColor = ParseColor(j, "pressColor", theme.button.pressColor);
        theme.button.textColor = ParseColor(j, "textColor", theme.button.textColor);
        theme.button.transitionTime = ParseFloat(j, "transitionTime", theme.button.transitionTime);
    }

    if (root.contains("slider")) {
        auto& j = root["slider"];
        theme.slider.trackColor = ParseColor(j, "trackColor", theme.slider.trackColor);
        theme.slider.fillColor = ParseColor(j, "fillColor", theme.slider.fillColor);
        theme.slider.handleColor = ParseColor(j, "handleColor", theme.slider.handleColor);
        theme.slider.handleActive = ParseColor(j, "handleActive", theme.slider.handleActive);
        theme.slider.trackHeight = ParseFloat(j, "trackHeight", theme.slider.trackHeight);
        theme.slider.handleWidth = ParseFloat(j, "handleWidth", theme.slider.handleWidth);
        theme.slider.handleHeight = ParseFloat(j, "handleHeight", theme.slider.handleHeight);
    }

    if (root.contains("progressBar")) {
        auto& j = root["progressBar"];
        theme.progressBar.backgroundColor = ParseColor(j, "backgroundColor", theme.progressBar.backgroundColor);
        theme.progressBar.fillColor = ParseColor(j, "fillColor", theme.progressBar.fillColor);
        theme.progressBar.warningColor = ParseColor(j, "warningColor", theme.progressBar.warningColor);
        theme.progressBar.dangerColor = ParseColor(j, "dangerColor", theme.progressBar.dangerColor);
    }

    if (root.contains("toggle")) {
        auto& j = root["toggle"];
        theme.toggle.offColor = ParseColor(j, "offColor", theme.toggle.offColor);
        theme.toggle.onColor = ParseColor(j, "onColor", theme.toggle.onColor);
        theme.toggle.handleColor = ParseColor(j, "handleColor", theme.toggle.handleColor);
    }

    if (root.contains("knob")) {
        auto& j = root["knob"];
        theme.knob.bgColor = ParseColor(j, "bgColor", theme.knob.bgColor);
        theme.knob.knobColor = ParseColor(j, "knobColor", theme.knob.knobColor);
        theme.knob.accentColor = ParseColor(j, "accentColor", theme.knob.accentColor);
        theme.knob.indicatorColor = ParseColor(j, "indicatorColor", theme.knob.indicatorColor);
    }

    if (root.contains("gauge")) {
        auto& j = root["gauge"];
        theme.gauge.bgColor = ParseColor(j, "bgColor", theme.gauge.bgColor);
        theme.gauge.trackColor = ParseColor(j, "trackColor", theme.gauge.trackColor);
        theme.gauge.fillColor = ParseColor(j, "fillColor", theme.gauge.fillColor);
        theme.gauge.warningColor = ParseColor(j, "warningColor", theme.gauge.warningColor);
        theme.gauge.dangerColor = ParseColor(j, "dangerColor", theme.gauge.dangerColor);
        theme.gauge.needleColor = ParseColor(j, "needleColor", theme.gauge.needleColor);
        theme.gauge.textColor = ParseColor(j, "textColor", theme.gauge.textColor);
    }

    if (root.contains("tabBar")) {
        auto& j = root["tabBar"];
        theme.tabBar.bgColor = ParseColor(j, "bgColor", theme.tabBar.bgColor);
        theme.tabBar.indicatorColor = ParseColor(j, "indicatorColor", theme.tabBar.indicatorColor);
        theme.tabBar.activeTextColor = ParseColor(j, "activeTextColor", theme.tabBar.activeTextColor);
        theme.tabBar.inactiveTextColor = ParseColor(j, "inactiveTextColor", theme.tabBar.inactiveTextColor);
    }

    if (root.contains("list")) {
        auto& j = root["list"];
        theme.list.bgColor = ParseColor(j, "bgColor", theme.list.bgColor);
        theme.list.altRowColor = ParseColor(j, "altRowColor", theme.list.altRowColor);
        theme.list.selectedColor = ParseColor(j, "selectedColor", theme.list.selectedColor);
        theme.list.textColor = ParseColor(j, "textColor", theme.list.textColor);
        theme.list.scrollbarColor = ParseColor(j, "scrollbarColor", theme.list.scrollbarColor);
    }

    spdlog::info("Theme loaded: {} from {}", theme.name, path);
    return theme;
}

} // namespace ThemeLoader
} // namespace CarHMI

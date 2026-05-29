#pragma once

#include "widget_factory.h"
#include <gui/widget.h>
#include <nlohmann/json.hpp>
#include <string>

namespace CarHMI::Runtime {

class SceneLoader {
public:
    explicit SceneLoader(WidgetFactory& factory);

    /// Load from a JSON file on disk. Returns root widget (caller owns), or nullptr on failure.
    GUI::Widget* LoadFromFile(const std::string& path);

    /// Load from a parsed JSON object.
    GUI::Widget* LoadFromJson(const nlohmann::json& root);

    const std::string& GetSceneName() const { return m_sceneName; }

private:
    GUI::Widget* ParseWidget(const nlohmann::json& node);

    WidgetFactory& m_factory;
    std::string m_sceneName;
    int m_autoId = 10000;
};

} // namespace CarHMI::Runtime

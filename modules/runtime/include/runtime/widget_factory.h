#pragma once

#include <gui/widget.h>
#include <nlohmann/json.hpp>
#include <functional>
#include <string>
#include <unordered_map>

namespace CarHMI::Runtime {

class WidgetFactory {
public:
    using Creator = std::function<GUI::Widget*(const nlohmann::json&, int id)>;

    void Register(const std::string& typeName, Creator creator);
    GUI::Widget* Create(const std::string& typeName, const nlohmann::json& props, int id);
    bool HasType(const std::string& typeName) const;

    void RegisterBuiltins();

    static WidgetFactory& Get() {
        static WidgetFactory instance;
        return instance;
    }

private:
    std::unordered_map<std::string, Creator> m_creators;
};

// Apply common widget properties from JSON (pos, size, margin, fillWidth, etc.)
void ApplyCommonProps(GUI::Widget* widget, const nlohmann::json& j);

} // namespace CarHMI::Runtime

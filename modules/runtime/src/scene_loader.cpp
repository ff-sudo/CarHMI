#include <runtime/scene_loader.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace CarHMI::Runtime {

using json = nlohmann::json;

SceneLoader::SceneLoader(WidgetFactory& factory)
    : m_factory(factory) {}

GUI::Widget* SceneLoader::LoadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        spdlog::error("SceneLoader: cannot open '{}'", path);
        return nullptr;
    }

    json root;
    try {
        root = json::parse(f);
    } catch (const std::exception& e) {
        spdlog::error("SceneLoader: JSON parse error in '{}': {}", path, e.what());
        return nullptr;
    }

    return LoadFromJson(root);
}

GUI::Widget* SceneLoader::LoadFromJson(const json& root) {
    m_sceneName = root.value("name", "Untitled");

    if (!root.contains("root")) {
        spdlog::error("SceneLoader: missing 'root' in scene '{}'", m_sceneName);
        return nullptr;
    }

    return ParseWidget(root["root"]);
}

GUI::Widget* SceneLoader::ParseWidget(const json& node) {
    if (!node.is_object()) return nullptr;

    std::string type = node.value("type", "");
    if (type.empty()) {
        spdlog::warn("SceneLoader: widget node missing 'type', skipping");
        return nullptr;
    }

    int id = node.value("id", m_autoId++);

    auto* widget = m_factory.Create(type, node, id);
    if (!widget) {
        spdlog::warn("SceneLoader: skipping unknown widget type '{}'", type);
        return nullptr;
    }

    // Recursively parse children
    if (node.contains("children") && node["children"].is_array()) {
        for (auto& childNode : node["children"]) {
            auto* child = ParseWidget(childNode);
            if (child) widget->AddChild(child);
        }
    }

    return widget;
}

} // namespace CarHMI::Runtime

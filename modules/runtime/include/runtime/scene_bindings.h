#pragma once

#include "data_model.h"
#include <gui/widget.h>
#include <core/property/binding.h>
#include <core/connection.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdio>

namespace CarHMI::Runtime {

/// SceneBindings: connects DataModel properties to Widget properties based on JSON declarations.
///
/// JSON format (within scene.json):
/// "bindings": [
///   { "source": "vehicle.speed", "target": 4, "property": "value" },
///   { "source": "vehicle.speed", "target": 3, "property": "text", "format": "Speed: %.0f" }
/// ]
///
/// "source" format: "modelName.propertyName"
/// "target": widget id
/// "property": widget property name (registered in Widget.Props())
/// "format" (optional): printf format for float → string conversion
class SceneBindings {
public:
    /// Establish bindings from JSON declarations.
    /// widgetIndex: id → Widget* map (from JsonScene::BuildWidgetIndex)
    void LoadFromJson(const nlohmann::json& bindingsArray,
                      const std::unordered_map<int, GUI::Widget*>& widgetIndex) {
        if (!bindingsArray.is_array()) return;

        for (auto& bj : bindingsArray) {
            std::string source = bj.value("source", "");
            int targetId = bj.value("target", 0);
            std::string propName = bj.value("property", "");
            std::string format = bj.value("format", "");

            if (source.empty() || targetId == 0 || propName.empty()) {
                spdlog::warn("SceneBindings: incomplete binding entry, skipping");
                continue;
            }

            // Parse "model.property" format
            auto dotPos = source.find('.');
            if (dotPos == std::string::npos) {
                spdlog::warn("SceneBindings: source '{}' missing dot separator", source);
                continue;
            }

            std::string modelName = source.substr(0, dotPos);
            std::string srcPropName = source.substr(dotPos + 1);

            // Find DataModel
            auto* model = DataModelRegistry::Get().Find(modelName);
            if (!model) {
                spdlog::warn("SceneBindings: DataModel '{}' not found", modelName);
                continue;
            }

            // Find source property
            auto* srcProp = model->GetFloat(srcPropName);
            if (!srcProp) {
                spdlog::warn("SceneBindings: property '{}' not found in model '{}'", srcPropName, modelName);
                continue;
            }

            // Find target widget
            auto it = widgetIndex.find(targetId);
            if (it == widgetIndex.end()) {
                spdlog::warn("SceneBindings: widget id {} not found", targetId);
                continue;
            }
            auto* widget = it->second;

            // Find target property on widget
            auto* targetBase = widget->Props().Find(propName);
            if (!targetBase) {
                spdlog::warn("SceneBindings: widget {} has no property '{}'", targetId, propName);
                continue;
            }

            // Determine binding type based on target property type and format
            if (!format.empty()) {
                // Format binding: float source → string target
                auto* targetStr = widget->Props().Get<std::string>(propName);
                if (targetStr) {
                    m_scope.BindFormat(*srcProp, *targetStr, format);
                    spdlog::debug("SceneBindings: {} → widget[{}].{} (format: {})", source, targetId, propName, format);
                } else {
                    spdlog::warn("SceneBindings: format binding requires string target, widget {}.{}", targetId, propName);
                }
            } else {
                // Direct binding: float → float
                auto* targetFloat = widget->Props().Get<float>(propName);
                if (targetFloat) {
                    m_scope.Bind<float>(*srcProp, *targetFloat);
                    spdlog::debug("SceneBindings: {} → widget[{}].{} (direct)", source, targetId, propName);
                } else {
                    spdlog::warn("SceneBindings: direct binding type mismatch for widget {}.{}", targetId, propName);
                }
            }
        }
    }

    void UnbindAll() {
        m_scope.UnbindAll();
    }

private:
    Core::BindingScope m_scope;
};

} // namespace CarHMI::Runtime

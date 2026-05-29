#pragma once

#include "scene_loader.h"
#include "widget_factory.h"
#include "scene_bindings.h"
#include "sim_data_source.h"
#include <core/scene/scene.h>
#include <gui/widget.h>
#include <gui/ui_context.h>
#include <rhi/batch_renderer2d.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace CarHMI::Runtime {

class JsonScene : public Core::Scene {
public:
    /// jsonPath: path to scene.json file
    /// ctx: UIContext for Update/Draw (non-owning, must outlive this scene)
    /// renderer: BatchRenderer2D for Draw (non-owning)
    JsonScene(const std::string& jsonPath, GUI::UIContext* ctx, RHI::BatchRenderer2D* renderer);

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    const char* GetName() const override;

    void Reload();
    GUI::Widget* FindById(int id);
    GUI::Widget* GetRoot() { return m_root; }

private:
    void BuildWidgetIndex(GUI::Widget* w);
    void SetupBindingsAndData(const nlohmann::json& root);

    std::string m_jsonPath;
    std::string m_sceneName;
    GUI::Widget* m_root = nullptr;
    GUI::UIContext* m_ctx;
    RHI::BatchRenderer2D* m_renderer;

    // Hot-reload
    std::filesystem::file_time_type m_lastModTime{};
    int m_frameCounter = 0;

    // Widget index
    std::unordered_map<int, GUI::Widget*> m_widgetIndex;

    // Bindings & simulation
    SceneBindings m_bindings;
    std::unique_ptr<SimDataSource> m_simSource;
};

} // namespace CarHMI::Runtime

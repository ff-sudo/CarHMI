#include <runtime/json_scene.h>
#include <runtime/data_model.h>
#include <gui/focus_manager.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

namespace CarHMI::Runtime {

JsonScene::JsonScene(const std::string& jsonPath, GUI::UIContext* ctx, RHI::BatchRenderer2D* renderer)
    : m_jsonPath(jsonPath), m_ctx(ctx), m_renderer(renderer) {}

void JsonScene::OnEnter() {
    WidgetFactory::Get().RegisterBuiltins();

    // Parse JSON once for both widget tree and bindings/data
    std::ifstream f(m_jsonPath);
    if (!f.is_open()) {
        spdlog::error("JsonScene: cannot open '{}'", m_jsonPath);
        return;
    }

    nlohmann::json rootJson;
    try {
        rootJson = nlohmann::json::parse(f);
    } catch (const std::exception& e) {
        spdlog::error("JsonScene: JSON parse error in '{}': {}", m_jsonPath, e.what());
        return;
    }

    SceneLoader loader(WidgetFactory::Get());
    m_root = loader.LoadFromJson(rootJson);
    m_sceneName = loader.GetSceneName();

    if (!m_root) {
        spdlog::error("JsonScene: failed to build widget tree from '{}'", m_jsonPath);
        return;
    }

    m_root->SubscribeThemeChange();
    GUI::FocusManager::Get().ClearAll();
    m_root->RegisterFocusRecursive();
    BuildWidgetIndex(m_root);

    // Setup data model, bindings, and simulation
    SetupBindingsAndData(rootJson);

    // Record file modification time for hot-reload
    try {
        m_lastModTime = std::filesystem::last_write_time(m_jsonPath);
    } catch (...) {}

    spdlog::info("JsonScene loaded: '{}' from '{}'", m_sceneName, m_jsonPath);
}

void JsonScene::OnExit() {
    m_bindings.UnbindAll();
    if (m_simSource) m_simSource->Stop();
    m_simSource.reset();

    delete m_root;
    m_root = nullptr;
    m_widgetIndex.clear();
}

void JsonScene::OnUpdate(float dt) {
    if (!m_root) return;
    m_root->Update(*m_ctx);

    // Hot-reload check every 120 frames (~2s at 60fps)
    m_frameCounter++;
    if (m_frameCounter % 120 == 0) {
        try {
            auto modTime = std::filesystem::last_write_time(m_jsonPath);
            if (modTime != m_lastModTime) {
                spdlog::info("JsonScene: detected change in '{}', reloading", m_jsonPath);
                Reload();
            }
        } catch (...) {}
    }
}

void JsonScene::OnDraw() {
    if (!m_root || !m_renderer) return;

    auto viewportSize = m_renderer->GetViewportSize();
    glm::mat4 proj = glm::ortho(0.0f, (float)viewportSize.x, (float)viewportSize.y, 0.0f, -1.0f, 1.0f);

    m_renderer->Begin(proj);
    m_root->Draw(*m_ctx);
    m_renderer->End();
}

const char* JsonScene::GetName() const {
    return m_sceneName.c_str();
}

void JsonScene::Reload() {
    // Cleanup
    m_bindings.UnbindAll();
    if (m_simSource) m_simSource->Stop();
    m_simSource.reset();
    delete m_root;
    m_root = nullptr;
    m_widgetIndex.clear();

    // Reload JSON
    std::ifstream f(m_jsonPath);
    if (!f.is_open()) return;

    nlohmann::json rootJson;
    try {
        rootJson = nlohmann::json::parse(f);
    } catch (...) { return; }

    SceneLoader loader(WidgetFactory::Get());
    m_root = loader.LoadFromJson(rootJson);
    m_sceneName = loader.GetSceneName();

    if (m_root) {
        m_root->SubscribeThemeChange();
        GUI::FocusManager::Get().ClearAll();
        m_root->RegisterFocusRecursive();
        BuildWidgetIndex(m_root);
        SetupBindingsAndData(rootJson);
    }

    try {
        m_lastModTime = std::filesystem::last_write_time(m_jsonPath);
    } catch (...) {}
}

void JsonScene::SetupBindingsAndData(const nlohmann::json& root) {
    // Setup DataModel from "dataModel" section
    if (root.contains("dataModel") && root["dataModel"].is_object()) {
        auto& dm = root["dataModel"];
        std::string modelName = dm.value("name", "default");
        auto* model = DataModelRegistry::Get().GetOrCreate(modelName);
        if (dm.contains("properties") && dm["properties"].is_object()) {
            model->LoadFromJson(dm["properties"]);
        }
    }

    // Setup SimDataSource from "dataSource" section
    if (root.contains("dataSource") && root["dataSource"].is_object()) {
        m_simSource = std::make_unique<SimDataSource>();
        m_simSource->LoadFromJson(root["dataSource"]);
        m_simSource->Start();
    }

    // Setup bindings from "bindings" section
    if (root.contains("bindings") && root["bindings"].is_array()) {
        m_bindings.LoadFromJson(root["bindings"], m_widgetIndex);
    }
}

GUI::Widget* JsonScene::FindById(int id) {
    auto it = m_widgetIndex.find(id);
    return (it != m_widgetIndex.end()) ? it->second : nullptr;
}

void JsonScene::BuildWidgetIndex(GUI::Widget* w) {
    if (!w) return;
    m_widgetIndex[w->GetID()] = w;
    for (auto* child : w->GetChildren())
        BuildWidgetIndex(child);
}

} // namespace CarHMI::Runtime

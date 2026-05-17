#pragma once

#include "platform/platform.h"
#include "core/event.h"
#include "renderer/batch_renderer2d.h"
#include "renderer/font.h"
#include "gui/ui_context.h"
#include <memory>
#include <string>

namespace CarHMI {

class SceneManager;

class Application {
public:
    Application(const WindowConfig& config = {});
    virtual ~Application();

    void Run();
    void Quit();

    Platform& GetPlatform() { return *m_platform; }
    BatchRenderer2D& GetRenderer() { return m_renderer; }
    Font& GetFont() { return m_font; }
    void ReloadFont(const std::string& path, float size);
    UIContext& GetUIContext() { return m_uiContext; }
    SceneManager& GetSceneManager() { return *m_sceneManager; }

    static Application& Get() { return *s_instance; }

protected:
    virtual void OnInit() {}
    virtual void OnShutdown() {}

private:
    void InitImGui();
    void ShutdownImGui();

    static Application* s_instance;

    std::unique_ptr<Platform> m_platform;
    std::unique_ptr<SceneManager> m_sceneManager;
    BatchRenderer2D m_renderer;
    Font m_font;
    UIContext m_uiContext;
    WindowConfig m_config;
    bool m_running = false;
    int m_frameCount = 0;
};

} // namespace CarHMI

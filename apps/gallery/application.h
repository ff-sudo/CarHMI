#pragma once

#include <pal/platform.h>
#include <core/event.h>
#include <core/connection.h>
#include <rhi/batch_renderer2d.h>
#include <rhi/font.h>
#include <gui/ui_context.h>
#include <memory>
#include <string>

namespace CarHMI::Core { class SceneManager; }

namespace CarHMI::Gallery {

class Application {
public:
    Application(const PAL::WindowConfig& config = {});
    virtual ~Application();

    void Run();
    void Quit();

    PAL::Platform& GetPlatform() { return *m_platform; }
    RHI::BatchRenderer2D& GetRenderer() { return m_renderer; }
    RHI::Font& GetFont() { return m_font; }
    void ReloadFont(const std::string& path, float size);
    GUI::UIContext& GetUIContext() { return m_uiContext; }
    Core::SceneManager& GetSceneManager() { return *m_sceneManager; }

    static Application& Get() { return *s_instance; }

protected:
    virtual void OnInit() {}
    virtual void OnShutdown() {}

private:
    void InitImGui();
    void ShutdownImGui();

    static Application* s_instance;

    std::unique_ptr<PAL::Platform> m_platform;
    std::unique_ptr<Core::SceneManager> m_sceneManager;
    RHI::BatchRenderer2D m_renderer;
    RHI::Font m_font;
    GUI::UIContext m_uiContext;
    PAL::WindowConfig m_config;
    bool m_running = false;
    int m_frameCount = 0;
    Core::ConnectionGroup m_connections;
};

} // namespace CarHMI::Gallery

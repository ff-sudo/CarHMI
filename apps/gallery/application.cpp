#include "application.h"
#include <core/animation/animation_manager.h>
#include <core/resource_manager.h>
#include <gui/style/theme_manager.h>
#include <gui/focus_manager.h>
#include <gui/i18n/i18n.h>
#include <core/event_bus.h>
#include <core/scene/scene_manager.h>
#include <pal/sdl2_factory.h>

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <SDL.h>

namespace CarHMI::Gallery {

Application* Application::s_instance = nullptr;

Application::Application(const PAL::WindowConfig& config)
    : m_config(config) {
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
}

void Application::Run() {
    m_platform = PAL::CreateSDL2Platform();
    if (!m_platform->Init()) return;
    if (!m_platform->CreateWindow(m_config)) return;

    if (!m_renderer.Init()) {
        spdlog::error("Failed to init renderer");
        return;
    }

    // Register resource loaders
    Core::ResourceManager::Get().RegisterLoader<RHI::Texture2D>(
        [](const std::string& path) {
            auto tex = std::make_shared<RHI::Texture2D>();
            if (!tex->LoadFromFile(path)) return std::shared_ptr<RHI::Texture2D>(nullptr);
            return tex;
        });

    m_font.LoadFromFile("resources/fonts/arial.ttf", 24.0f);

    InitImGui();

    m_uiContext.Init(&m_renderer, &m_font);

    GUI::FocusManager::Get().Init();

    m_sceneManager = std::make_unique<Core::SceneManager>();

    m_connections.Add(Core::EventBus::Get().Subscribe<Core::WindowCloseEvent>(
        [this](const Core::WindowCloseEvent&) {
            m_running = false;
        }));

    m_connections.Add(Core::EventBus::Get().Subscribe<GUI::LanguageChangedEvent>(
        [this](const GUI::LanguageChangedEvent& e) {
            if (e.fontPath && e.fontPath[0] != '\0') {
                ReloadFont(e.fontPath, 24.0f);
            }
        }));

    OnInit();

    m_running = true;
    Uint64 lastTime = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();

    spdlog::info("Application main loop started");

    while (m_running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - lastTime) / (float)freq;
        lastTime = now;

        Core::EventBus::Get().FlushDeferred();
        Core::EventBus::Get().Post(Core::FrameBeginEvent{});
        m_platform->PollEvents();
        if (m_platform->ShouldClose()) m_running = false;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        Core::EventBus::Get().Post(Core::AppTickEvent{dt});
        Core::AnimationManager::Get().Update(dt);

        m_frameCount++;
        if (m_frameCount % 60 == 0)
            GUI::ThemeManager::Get().CheckHotReload();

        auto& theme = GUI::ThemeManager::Get().GetTheme();

        m_sceneManager->Update(dt);

        auto winSize = m_platform->GetWindowSize();
        glViewport(0, 0, winSize.x, winSize.y);
        m_renderer.SetViewportSize(winSize.x, winSize.y);
        glClearColor(theme.app.clearColor.r, theme.app.clearColor.g,
                     theme.app.clearColor.b, theme.app.clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_sceneManager->Draw();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_platform->SwapBuffers();
    }

    OnShutdown();

    m_sceneManager.reset();
    ShutdownImGui();
    m_renderer.Shutdown();
    m_platform->Shutdown();

    spdlog::info("Application shutdown complete");
}

void Application::ReloadFont(const std::string& path, float size) {
    m_font = RHI::Font();
    m_font.LoadFromFile(path, size);
    spdlog::info("Font reloaded: {} size={}", path, size);
}

void Application::Quit() {
    m_running = false;
}

void Application::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    SDL_Window* window = (SDL_Window*)m_platform->GetNativeWindowHandle();
    SDL_GLContext ctx = (SDL_GLContext)m_platform->GetGLContext();
    ImGui_ImplSDL2_InitForOpenGL(window, ctx);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_connections.Add(Core::EventBus::Get().Subscribe<Core::SDLRawEvent>(
        [](const Core::SDLRawEvent& e) {
            ImGui_ImplSDL2_ProcessEvent(e.event);
        }));
}

void Application::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

} // namespace CarHMI::Gallery

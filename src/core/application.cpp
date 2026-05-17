#include "application.h"
#include "animation/animation_manager.h"
#include "gui/style/theme_manager.h"
#include "gui/focus_manager.h"
#include "core/event_bus.h"
#include "scene/scene_manager.h"
#include "platform/sdl2/sdl2_platform.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

namespace CarHMI {

Application* Application::s_instance = nullptr;

Application::Application(const WindowConfig& config)
    : m_config(config) {
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
}

void Application::Run() {
    // Platform
    m_platform = std::make_unique<SDL2Platform>();
    if (!m_platform->Init()) return;
    if (!m_platform->CreateWindow(m_config)) return;

    // Renderer
    if (!m_renderer.Init()) {
        spdlog::error("Failed to init renderer");
        return;
    }

    // Font
    m_font.LoadFromFile("resources/fonts/arial.ttf", 24.0f);

    // ImGui
    InitImGui();

    // UI Context (global, subscribed once to EventBus)
    m_uiContext.Init(&m_renderer, &m_font);

    // Focus manager
    FocusManager::Get().Init();

    // Scene manager
    m_sceneManager = std::make_unique<SceneManager>();

    // Subscribe to window close
    EventBus::Get().subscribe<WindowCloseEvent>([this](const WindowCloseEvent&) {
        m_running = false;
    });

    // User init
    OnInit();

    m_running = true;
    Uint64 lastTime = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();

    spdlog::info("Application main loop started");

    while (m_running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - lastTime) / (float)freq;
        lastTime = now;

        // Poll
        EventBus::Get().post(FrameBeginEvent{});
        m_platform->PollEvents();
        if (m_platform->ShouldClose()) m_running = false;

        // ImGui new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Tick
        EventBus::Get().post(AppTickEvent{dt});
        AnimationManager::Get().Update(dt);

        // Hot reload check (~every 60 frames)
        m_frameCount++;
        if (m_frameCount % 60 == 0)
            ThemeManager::Get().CheckHotReload();

        // Apply theme clear color
        auto& theme = ThemeManager::Get().GetTheme();

        // Scene update & draw
        m_sceneManager->Update(dt);

        auto winSize = m_platform->GetWindowSize();
        glViewport(0, 0, winSize.x, winSize.y);
        glClearColor(theme.app.clearColor.r, theme.app.clearColor.g,
                     theme.app.clearColor.b, theme.app.clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Scene handles all rendering (both 3D and 2D)
        m_sceneManager->Draw();

        // ImGui render
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
    m_font = Font();
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

    // Subscribe to raw SDL events for ImGui
    EventBus::Get().subscribe<SDLRawEvent>([](const SDLRawEvent& e) {
        ImGui_ImplSDL2_ProcessEvent(e.event);
    });
}

void Application::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

} // namespace CarHMI

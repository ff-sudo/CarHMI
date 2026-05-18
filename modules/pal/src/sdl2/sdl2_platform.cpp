#include "sdl2_platform.h"
#include <core/event_bus.h>
#include <core/event.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

namespace CarHMI::PAL {

bool SDL2Platform::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    spdlog::info("SDL2 platform initialized");
    return true;
}

void SDL2Platform::Shutdown() {
    DestroyWindow();
    SDL_Quit();
    spdlog::info("SDL2 platform shutdown");
}

bool SDL2Platform::CreateWindow(const WindowConfig& config) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
    if (config.resizable) flags |= SDL_WINDOW_RESIZABLE;
    if (config.fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    m_window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.width, config.height, flags
    );
    if (!m_window) {
        spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(config.vsync ? 1 : 0);

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    if (version == 0) {
        spdlog::error("GLAD initialization failed");
        return false;
    }

    spdlog::info("Window created: {}x{}, OpenGL {}.{}",
                 config.width, config.height,
                 GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    return true;
}

void SDL2Platform::DestroyWindow() {
    if (m_glContext) { SDL_GL_DeleteContext(m_glContext); m_glContext = nullptr; }
    if (m_window) { SDL_DestroyWindow(m_window); m_window = nullptr; }
}

void SDL2Platform::PollEvents() {
    using namespace CarHMI::Core;

    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        EventBus::Get().post(SDLRawEvent{&sdlEvent});

        switch (sdlEvent.type) {
        case SDL_QUIT:
            m_shouldClose = true;
            EventBus::Get().post(WindowCloseEvent{});
            break;
        case SDL_WINDOWEVENT:
            if (sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE)
                m_shouldClose = true;
            if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED)
                EventBus::Get().post(WindowResizeEvent{sdlEvent.window.data1, sdlEvent.window.data2});
            break;
        case SDL_MOUSEMOTION:
            EventBus::Get().post(MouseMoveEvent{(float)sdlEvent.motion.x, (float)sdlEvent.motion.y});
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (sdlEvent.button.button == SDL_BUTTON_LEFT)
                EventBus::Get().post(MouseButtonEvent{MouseButton::Left, true,
                    (float)sdlEvent.button.x, (float)sdlEvent.button.y});
            break;
        case SDL_MOUSEBUTTONUP:
            if (sdlEvent.button.button == SDL_BUTTON_LEFT)
                EventBus::Get().post(MouseButtonEvent{MouseButton::Left, false,
                    (float)sdlEvent.button.x, (float)sdlEvent.button.y});
            break;
        case SDL_KEYDOWN:
            EventBus::Get().post(KeyEvent{sdlEvent.key.keysym.scancode, true});
            break;
        case SDL_KEYUP:
            EventBus::Get().post(KeyEvent{sdlEvent.key.keysym.scancode, false});
            break;
        case SDL_MOUSEWHEEL:
            EventBus::Get().post(MouseScrollEvent{(float)sdlEvent.wheel.x, (float)sdlEvent.wheel.y});
            break;
        }
    }
}

void SDL2Platform::SwapBuffers() {
    SDL_GL_SwapWindow(m_window);
}

void SDL2Platform::SetWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(m_window, title.c_str());
}

glm::ivec2 SDL2Platform::GetWindowSize() const {
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    return {w, h};
}

void SDL2Platform::SetWindowSize(int w, int h) {
    SDL_SetWindowSize(m_window, w, h);
}

void SDL2Platform::SetFullscreen(bool fs) {
    SDL_SetWindowFullscreen(m_window, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

} // namespace CarHMI::PAL

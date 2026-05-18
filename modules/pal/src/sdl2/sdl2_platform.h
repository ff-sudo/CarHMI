#pragma once

#include <pal/platform.h>
#include <SDL.h>

namespace CarHMI::PAL {

class SDL2Platform : public Platform {
public:
    bool Init() override;
    void Shutdown() override;

    bool CreateWindow(const WindowConfig& config) override;
    void DestroyWindow() override;

    void PollEvents() override;
    void SwapBuffers() override;

    void SetWindowTitle(const std::string& title) override;
    glm::ivec2 GetWindowSize() const override;
    void SetWindowSize(int w, int h) override;
    void SetFullscreen(bool fs) override;

    bool ShouldClose() const override { return m_shouldClose; }

    void* GetNativeWindowHandle() const override { return m_window; }
    void* GetGLContext() const override { return m_glContext; }

private:
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    bool m_shouldClose = false;
};

} // namespace CarHMI::PAL

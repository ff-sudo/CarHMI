#pragma once

#include <string>
#include <glm/glm.hpp>

namespace CarHMI::PAL {

struct WindowConfig {
    std::string title = "CarHMI";
    int width = 1280;
    int height = 720;
    bool resizable = true;
    bool fullscreen = false;
    bool vsync = true;
};

class Platform {
public:
    virtual ~Platform() = default;

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

    virtual bool CreateWindow(const WindowConfig& config) = 0;
    virtual void DestroyWindow() = 0;

    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;

    virtual void SetWindowTitle(const std::string& title) = 0;
    virtual glm::ivec2 GetWindowSize() const = 0;
    virtual void SetWindowSize(int w, int h) = 0;
    virtual void SetFullscreen(bool fs) = 0;

    virtual bool ShouldClose() const = 0;

    virtual void* GetNativeWindowHandle() const = 0;
    virtual void* GetGLContext() const = 0;
};

} // namespace CarHMI::PAL

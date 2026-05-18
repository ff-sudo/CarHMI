#pragma once

#include <SDL.h>

namespace CarHMI::Core {

// === Window Events ===
struct WindowCloseEvent {};

struct WindowResizeEvent {
    int width;
    int height;
};

// === Mouse Events ===
enum class MouseButton { Left, Right, Middle };

struct MouseMoveEvent {
    float x;
    float y;
};

struct MouseButtonEvent {
    MouseButton button;
    bool pressed;
    float x;
    float y;
};

struct MouseScrollEvent {
    float xOffset;
    float yOffset;
};

// === Keyboard Events ===
struct KeyEvent {
    int scancode;
    bool pressed;
};

// === SDL Raw Event (for ImGui passthrough) ===
struct SDLRawEvent {
    const SDL_Event* event;
};

// === App Events ===
struct FrameBeginEvent {};

struct AppTickEvent {
    float deltaTime;
};

struct ThemeChangedEvent {
    const char* themeName;
};

} // namespace CarHMI::Core

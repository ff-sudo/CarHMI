#pragma once

namespace CarHMI::GUI {

struct Insets {
    float top = 0, right = 0, bottom = 0, left = 0;

    Insets() = default;
    Insets(float all) : top(all), right(all), bottom(all), left(all) {}
    Insets(float vertical, float horizontal)
        : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
    Insets(float t, float r, float b, float l)
        : top(t), right(r), bottom(b), left(l) {}

    float horizontal() const { return left + right; }
    float vertical() const { return top + bottom; }
};

} // namespace CarHMI::GUI

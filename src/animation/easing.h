#pragma once

#include <cmath>
#include <functional>

namespace CarHMI {

using EasingFunc = std::function<float(float)>;

namespace Easing {

inline float Linear(float t) { return t; }

inline float InQuad(float t) { return t * t; }
inline float OutQuad(float t) { return t * (2.0f - t); }
inline float InOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

inline float InCubic(float t) { return t * t * t; }
inline float OutCubic(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
inline float InOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

inline float InExpo(float t) { return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f)); }
inline float OutExpo(float t) { return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t); }
inline float InOutExpo(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return t < 0.5f
        ? 0.5f * powf(2.0f, 20.0f * t - 10.0f)
        : 1.0f - 0.5f * powf(2.0f, -20.0f * t + 10.0f);
}

inline float InBack(float t) {
    constexpr float s = 1.70158f;
    return t * t * ((s + 1.0f) * t - s);
}
inline float OutBack(float t) {
    constexpr float s = 1.70158f;
    float u = t - 1.0f;
    return u * u * ((s + 1.0f) * u + s) + 1.0f;
}

inline float OutBounce(float t) {
    if (t < 1.0f / 2.75f)
        return 7.5625f * t * t;
    if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    }
    if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    }
    t -= 2.625f / 2.75f;
    return 7.5625f * t * t + 0.984375f;
}
inline float InBounce(float t) { return 1.0f - OutBounce(1.0f - t); }

inline float InElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return -powf(2.0f, 10.0f * (t - 1.0f)) * sinf((t - 1.1f) * 5.0f * 3.14159265f);
}
inline float OutElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return powf(2.0f, -10.0f * t) * sinf((t - 0.1f) * 5.0f * 3.14159265f) + 1.0f;
}

} // namespace Easing
} // namespace CarHMI

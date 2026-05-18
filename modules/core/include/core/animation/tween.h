#pragma once

#include "animation.h"
#include "easing.h"
#include <core/property/property.h>
#include <glm/glm.hpp>
#include <functional>

namespace CarHMI::Core {

template<typename T>
class Tween : public Animation {
public:
    Tween(T* target, T from, T to, float duration, EasingFunc easing = Easing::Linear)
        : m_target(target), m_from(from), m_to(to)
        , m_duration(duration), m_easing(std::move(easing)) {}

    void Update(float dt) override {
        if (m_finished) return;

        m_elapsed += dt;
        float t = m_elapsed / m_duration;
        if (t >= 1.0f) {
            t = 1.0f;
            m_finished = true;
        }

        float eased = m_easing(t);
        *m_target = Lerp(m_from, m_to, eased);

        if (m_finished) FireComplete();
    }

    bool IsFinished() const override { return m_finished; }

    void Reset() override {
        m_elapsed = 0.0f;
        m_finished = false;
    }

private:
    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    static glm::vec2 Lerp(glm::vec2 a, glm::vec2 b, float t) { return a + (b - a) * t; }
    static glm::vec4 Lerp(glm::vec4 a, glm::vec4 b, float t) { return a + (b - a) * t; }

    T* m_target;
    T m_from;
    T m_to;
    float m_duration;
    float m_elapsed = 0.0f;
    bool m_finished = false;
    EasingFunc m_easing;
};

template<typename T>
class PropertyTween : public Animation {
public:
    PropertyTween(Property<T>& prop, T from, T to, float duration, EasingFunc easing = Easing::Linear)
        : m_prop(prop), m_from(from), m_to(to)
        , m_duration(duration), m_easing(std::move(easing)) {}

    void Update(float dt) override {
        if (m_finished) return;

        m_elapsed += dt;
        float t = m_elapsed / m_duration;
        if (t >= 1.0f) {
            t = 1.0f;
            m_finished = true;
        }

        float eased = m_easing(t);
        T val = Lerp(m_from, m_to, eased);
        m_prop.Set(val);

        if (m_finished) FireComplete();
    }

    bool IsFinished() const override { return m_finished; }

    void Reset() override {
        m_elapsed = 0.0f;
        m_finished = false;
    }

private:
    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    static glm::vec2 Lerp(glm::vec2 a, glm::vec2 b, float t) { return a + (b - a) * t; }
    static glm::vec4 Lerp(glm::vec4 a, glm::vec4 b, float t) { return a + (b - a) * t; }

    Property<T>& m_prop;
    T m_from;
    T m_to;
    float m_duration;
    float m_elapsed = 0.0f;
    bool m_finished = false;
    EasingFunc m_easing;
};

using TweenFloat = Tween<float>;
using TweenVec2 = Tween<glm::vec2>;
using TweenVec4 = Tween<glm::vec4>;

} // namespace CarHMI::Core

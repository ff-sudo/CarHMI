#pragma once

#include "animation.h"
#include "tween.h"
#include "sequence.h"
#include "easing.h"
#include <vector>
#include <memory>

namespace CarHMI::Core {

class AnimationManager {
public:
    static AnimationManager& Get() {
        static AnimationManager instance;
        return instance;
    }

    void Update(float dt) {
        for (int i = (int)m_animations.size() - 1; i >= 0; i--) {
            m_animations[i]->Update(dt);
            if (m_animations[i]->IsFinished()) {
                m_animations.erase(m_animations.begin() + i);
            }
        }
    }

    Animation* Add(std::unique_ptr<Animation> anim) {
        auto* ptr = anim.get();
        m_animations.push_back(std::move(anim));
        return ptr;
    }

    template<typename T>
    Tween<T>* TweenTo(T* target, T to, float duration, EasingFunc easing = Easing::Linear) {
        T from = *target;
        auto tween = std::make_unique<Tween<T>>(target, from, to, duration, std::move(easing));
        auto* ptr = tween.get();
        m_animations.push_back(std::move(tween));
        return ptr;
    }

    template<typename T>
    Tween<T>* TweenFromTo(T* target, T from, T to, float duration, EasingFunc easing = Easing::Linear) {
        auto tween = std::make_unique<Tween<T>>(target, from, to, duration, std::move(easing));
        auto* ptr = tween.get();
        m_animations.push_back(std::move(tween));
        return ptr;
    }

    template<typename T>
    PropertyTween<T>* TweenPropertyTo(Property<T>& prop, T to, float duration, EasingFunc easing = Easing::Linear) {
        T from = prop.Get();
        auto tween = std::make_unique<PropertyTween<T>>(prop, from, to, duration, std::move(easing));
        auto* ptr = tween.get();
        m_animations.push_back(std::move(tween));
        return ptr;
    }

    void Clear() { m_animations.clear(); }
    int ActiveCount() const { return (int)m_animations.size(); }

private:
    AnimationManager() = default;
    std::vector<std::unique_ptr<Animation>> m_animations;
};

} // namespace CarHMI::Core

#pragma once

#include "scene.h"
#include <vector>
#include <memory>
#include <functional>

namespace CarHMI::Core {

enum class TransitionType { None, SlideLeft, SlideRight, FadeIn, FadeOut };

class SceneManager {
public:
    void Push(std::unique_ptr<Scene> scene, TransitionType transition = TransitionType::SlideLeft);
    void Pop(TransitionType transition = TransitionType::SlideRight);
    void Replace(std::unique_ptr<Scene> scene, TransitionType transition = TransitionType::FadeIn);

    void Update(float dt);
    void Draw();

    Scene* Current() const;
    int StackSize() const { return (int)m_stack.size(); }

    bool IsTransitioning() const { return m_transitioning; }
    float GetTransitionOffset() const { return m_transitionOffset; }
    float GetTransitionAlpha() const { return m_transitionAlpha; }

private:
    void StartTransition(TransitionType type, float duration = 0.3f);

    std::vector<std::unique_ptr<Scene>> m_stack;

    bool m_transitioning = false;
    float m_transitionOffset = 0.0f;
    float m_transitionAlpha = 1.0f;
};

} // namespace CarHMI::Core

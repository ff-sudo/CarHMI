#include <core/scene/scene_manager.h>
#include <core/animation/animation_manager.h>
#include <core/animation/tween.h>
#include <core/animation/easing.h>
#include <spdlog/spdlog.h>

namespace CarHMI::Core {

void SceneManager::Push(std::unique_ptr<Scene> scene, TransitionType transition) {
    if (!m_stack.empty())
        m_stack.back()->OnPause();

    spdlog::info("Scene push: {}", scene->GetName());
    scene->OnEnter();
    m_stack.push_back(std::move(scene));

    StartTransition(transition);
}

void SceneManager::Pop(TransitionType transition) {
    // Defer the actual pop to after Update/OnImGui finishes
    // to avoid use-after-free when Pop is called from OnImGui
    m_pendingPop = true;
    m_pendingPopTransition = transition;
}

void SceneManager::ProcessPendingPop() {
    if (!m_pendingPop) return;
    m_pendingPop = false;

    if (m_stack.empty()) return;

    spdlog::info("Scene pop: {}", m_stack.back()->GetName());
    m_stack.back()->OnExit();
    m_stack.pop_back();

    if (!m_stack.empty()) {
        m_stack.back()->OnResume();
        StartTransition(m_pendingPopTransition);
    }
}

void SceneManager::Replace(std::unique_ptr<Scene> scene, TransitionType transition) {
    if (!m_stack.empty()) {
        spdlog::info("Scene replace: {} -> {}", m_stack.back()->GetName(), scene->GetName());
        m_stack.back()->OnExit();
        m_stack.pop_back();
    }
    scene->OnEnter();
    m_stack.push_back(std::move(scene));
    StartTransition(transition);
}

void SceneManager::Update(float dt) {
    if (!m_stack.empty()) {
        m_stack.back()->OnUpdate(dt);
        m_stack.back()->OnImGui();
    }
    ProcessPendingPop();
}

void SceneManager::Draw() {
    if (!m_stack.empty())
        m_stack.back()->OnDraw();
}

Scene* SceneManager::Current() const {
    if (m_stack.empty()) return nullptr;
    return m_stack.back().get();
}

void SceneManager::StartTransition(TransitionType type, float duration) {
    if (type == TransitionType::None) return;

    m_transitioning = true;

    switch (type) {
    case TransitionType::SlideLeft:
        m_transitionOffset = 400.0f;
        AnimationManager::Get().TweenTo(&m_transitionOffset, 0.0f, duration, Easing::OutCubic)
            ->SetOnComplete([this]() { m_transitioning = false; });
        break;
    case TransitionType::SlideRight:
        m_transitionOffset = -400.0f;
        AnimationManager::Get().TweenTo(&m_transitionOffset, 0.0f, duration, Easing::OutCubic)
            ->SetOnComplete([this]() { m_transitioning = false; });
        break;
    case TransitionType::FadeIn:
        m_transitionAlpha = 0.0f;
        AnimationManager::Get().TweenTo(&m_transitionAlpha, 1.0f, duration, Easing::OutQuad)
            ->SetOnComplete([this]() { m_transitioning = false; });
        break;
    case TransitionType::FadeOut:
        m_transitionAlpha = 1.0f;
        AnimationManager::Get().TweenTo(&m_transitionAlpha, 0.0f, duration, Easing::InQuad)
            ->SetOnComplete([this]() { m_transitioning = false; });
        break;
    default:
        m_transitioning = false;
        break;
    }
}

} // namespace CarHMI::Core

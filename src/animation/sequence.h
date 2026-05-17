#pragma once

#include "animation.h"
#include <vector>
#include <memory>

namespace CarHMI {

class Delay : public Animation {
public:
    explicit Delay(float duration) : m_duration(duration) {}

    void Update(float dt) override {
        if (m_finished) return;
        m_elapsed += dt;
        if (m_elapsed >= m_duration) {
            m_finished = true;
            FireComplete();
        }
    }

    bool IsFinished() const override { return m_finished; }
    void Reset() override { m_elapsed = 0.0f; m_finished = false; }

private:
    float m_duration;
    float m_elapsed = 0.0f;
    bool m_finished = false;
};

class Sequence : public Animation {
public:
    Sequence& Add(std::unique_ptr<Animation> anim) {
        m_animations.push_back(std::move(anim));
        return *this;
    }

    void Update(float dt) override {
        if (m_finished || m_animations.empty()) return;

        while (dt > 0.0f && m_current < (int)m_animations.size()) {
            auto& anim = m_animations[m_current];
            anim->Update(dt);
            if (anim->IsFinished()) {
                m_current++;
            } else {
                return;
            }
        }

        m_finished = (m_current >= (int)m_animations.size());
        if (m_finished) FireComplete();
    }

    bool IsFinished() const override { return m_finished; }

    void Reset() override {
        m_current = 0;
        m_finished = false;
        for (auto& a : m_animations) a->Reset();
    }

private:
    std::vector<std::unique_ptr<Animation>> m_animations;
    int m_current = 0;
    bool m_finished = false;
};

class Parallel : public Animation {
public:
    Parallel& Add(std::unique_ptr<Animation> anim) {
        m_animations.push_back(std::move(anim));
        return *this;
    }

    void Update(float dt) override {
        if (m_finished) return;

        bool allDone = true;
        for (auto& anim : m_animations) {
            if (!anim->IsFinished()) {
                anim->Update(dt);
                if (!anim->IsFinished()) allDone = false;
            }
        }

        if (allDone) {
            m_finished = true;
            FireComplete();
        }
    }

    bool IsFinished() const override { return m_finished; }

    void Reset() override {
        m_finished = false;
        for (auto& a : m_animations) a->Reset();
    }

private:
    std::vector<std::unique_ptr<Animation>> m_animations;
    bool m_finished = false;
};

} // namespace CarHMI

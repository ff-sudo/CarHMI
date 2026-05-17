#pragma once

#include <functional>

namespace CarHMI {

class Animation {
public:
    virtual ~Animation() = default;

    virtual void Update(float dt) = 0;
    virtual bool IsFinished() const = 0;
    virtual void Reset() = 0;

    void SetOnComplete(std::function<void()> cb) { m_onComplete = std::move(cb); }

protected:
    void FireComplete() { if (m_onComplete) m_onComplete(); }

    std::function<void()> m_onComplete;
};

} // namespace CarHMI

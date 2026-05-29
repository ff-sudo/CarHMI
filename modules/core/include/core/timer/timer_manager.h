#pragma once

#include <core/connection.h>
#include <functional>
#include <vector>
#include <memory>

namespace CarHMI::Core {

class TimerManager {
public:
    TimerManager() = default;

    TimerManager(const TimerManager&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;

    /// Set the global instance so Get() returns the same instance the main loop ticks.
    static void SetGlobalInstance(TimerManager* inst) { s_globalInstance = inst; }

    static TimerManager& Get() {
        if (s_globalInstance) return *s_globalInstance;
        static TimerManager instance;
        return instance;
    }

    // setTimeout: 延迟 delayMs 毫秒后执行一次
    Connection SetTimeout(std::function<void()> callback, int delayMs) {
        int id = m_nextId++;
        m_timers.push_back({id, std::move(callback), delayMs, 0.0f, false, false});
        return Connection([this, id]() { Cancel(id); });
    }

    // setInterval: 每隔 intervalMs 毫秒重复执行
    Connection SetInterval(std::function<void()> callback, int intervalMs) {
        int id = m_nextId++;
        m_timers.push_back({id, std::move(callback), intervalMs, 0.0f, true, false});
        return Connection([this, id]() { Cancel(id); });
    }

    // 主循环每帧调用，dt 为秒
    void Update(float dt) {
        float dtMs = dt * 1000.0f;

        for (int i = (int)m_timers.size() - 1; i >= 0; i--) {
            auto& t = m_timers[i];
            if (t.cancelled) {
                m_timers.erase(m_timers.begin() + i);
                continue;
            }

            t.elapsedMs += dtMs;

            if (t.elapsedMs >= (float)t.intervalMs) {
                t.callback();

                if (t.repeating) {
                    t.elapsedMs -= (float)t.intervalMs;
                } else {
                    m_timers.erase(m_timers.begin() + i);
                }
            }
        }
    }

    void ClearAll() {
        m_timers.clear();
    }

    int ActiveCount() const {
        return (int)m_timers.size();
    }

private:
    void Cancel(int id) {
        for (auto& t : m_timers) {
            if (t.id == id) {
                t.cancelled = true;
                return;
            }
        }
    }

    struct TimerEntry {
        int id;
        std::function<void()> callback;
        int intervalMs;
        float elapsedMs;
        bool repeating;
        bool cancelled;
    };

    static TimerManager* s_globalInstance;
    std::vector<TimerEntry> m_timers;
    int m_nextId = 1;
};

inline TimerManager* TimerManager::s_globalInstance = nullptr;

} // namespace CarHMI::Core

#pragma once

#include <core/connection.h>
#include <functional>
#include <vector>
#include <memory>

namespace CarHMI::Core {

class TimerManager {
public:
    static TimerManager& Get() {
        static TimerManager instance;
        return instance;
    }

    TimerManager(const TimerManager&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;

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
    TimerManager() = default;

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

    std::vector<TimerEntry> m_timers;
    int m_nextId = 1;
};

} // namespace CarHMI::Core

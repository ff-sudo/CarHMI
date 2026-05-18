#pragma once

#include "connection.h"
#include <eventpp/callbacklist.h>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <any>
#include <vector>
#include <mutex>

namespace CarHMI::Core {

class EventBus {
public:
    static EventBus& Get() {
        static EventBus instance;
        return instance;
    }

    EventBus() = default;
    ~EventBus() = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    template<typename E>
    using Listener = std::function<void(const E&)>;

    template<typename E>
    Connection Subscribe(Listener<E> callback) {
        auto& list = getOrCreate<E>();
        auto handle = list.append(std::move(callback));
        auto key = std::type_index(typeid(E));

        return Connection([this, key, handle]() {
            auto it = m_dispatchers.find(key);
            if (it != m_dispatchers.end()) {
                auto& cbList = std::any_cast<CallbackListOf<E>&>(it->second);
                cbList.remove(handle);
            }
        });
    }

    template<typename E>
    void Post(const E& event) {
        auto it = m_dispatchers.find(typeid(E));
        if (it != m_dispatchers.end()) {
            auto& list = std::any_cast<CallbackListOf<E>&>(it->second);
            list(event);
        }
    }

    template<typename E>
    void PostDeferred(E event) {
        std::lock_guard<std::mutex> lock(m_deferredMutex);
        m_deferredQueue.push_back([this, e = std::move(event)]() {
            Post(e);
        });
    }

    void FlushDeferred() {
        std::vector<std::function<void()>> batch;
        {
            std::lock_guard<std::mutex> lock(m_deferredMutex);
            batch.swap(m_deferredQueue);
        }
        for (auto& fn : batch)
            fn();
    }

    // Backward compatibility (deprecated, will be removed)
    template<typename E>
    [[deprecated("Use Subscribe() which returns Connection")]]
    Connection subscribe(Listener<E> callback) {
        return Subscribe<E>(std::move(callback));
    }

    template<typename E>
    void post(const E& event) { Post(event); }

private:
    template<typename E>
    using CallbackListOf = eventpp::CallbackList<void(const E&)>;

    template<typename E>
    CallbackListOf<E>& getOrCreate() {
        auto key = std::type_index(typeid(E));
        auto it = m_dispatchers.find(key);
        if (it == m_dispatchers.end()) {
            auto [inserted, _] = m_dispatchers.emplace(key, CallbackListOf<E>{});
            return std::any_cast<CallbackListOf<E>&>(inserted->second);
        }
        return std::any_cast<CallbackListOf<E>&>(it->second);
    }

    std::unordered_map<std::type_index, std::any> m_dispatchers;
    std::mutex m_deferredMutex;
    std::vector<std::function<void()>> m_deferredQueue;
};

// Backward compatibility: SubscriptionHandle is now Connection
using SubscriptionHandle = Connection;

} // namespace CarHMI::Core

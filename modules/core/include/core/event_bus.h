#pragma once

#include <eventpp/callbacklist.h>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <any>

namespace CarHMI::Core {

class SubscriptionHandle {
public:
    SubscriptionHandle() = default;
    SubscriptionHandle(std::function<void()> unsub) : m_unsub(std::move(unsub)) {}

    void Unsubscribe() {
        if (m_unsub) { m_unsub(); m_unsub = nullptr; }
    }

private:
    std::function<void()> m_unsub;
};

class EventBus {
public:
    static EventBus& Get() {
        static EventBus instance;
        return instance;
    }

    template<typename E>
    using Listener = std::function<void(const E&)>;

    template<typename E>
    SubscriptionHandle subscribe(Listener<E> callback) {
        auto& list = getOrCreate<E>();
        auto handle = list.append(std::move(callback));

        auto key = std::type_index(typeid(E));
        return SubscriptionHandle([this, key, handle]() {
            auto it = m_dispatchers.find(key);
            if (it != m_dispatchers.end()) {
                auto& cbList = std::any_cast<CallbackListOf<E>&>(it->second);
                cbList.remove(handle);
            }
        });
    }

    template<typename E>
    void post(const E& event) {
        auto it = m_dispatchers.find(typeid(E));
        if (it != m_dispatchers.end()) {
            auto& list = std::any_cast<CallbackListOf<E>&>(it->second);
            list(event);
        }
    }

private:
    EventBus() = default;

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
};

} // namespace CarHMI::Core

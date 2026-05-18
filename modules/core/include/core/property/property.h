#pragma once

#include "property_base.h"
#include <core/connection.h>
#include <functional>
#include <vector>
#include <mutex>
#include <sstream>
#include <glm/glm.hpp>

namespace CarHMI::Core {

template<typename T>
class Property : public PropertyBase {
public:
    using ChangedCallback = std::function<void(const T& oldVal, const T& newVal)>;

    Property(const std::string& name, T initial = T{})
        : PropertyBase(name, typeid(T)), m_value(initial) {}

    const T& Get() const { return m_value; }
    operator const T&() const { return m_value; }

    void Set(const T& value) {
        if (m_value == value) return;
        T old = m_value;
        m_value = value;
        NotifyChanged(old, m_value);
    }

    void SetDeferred(const T& value) {
        if (!m_dirty) m_oldValueForFlush = m_value;
        m_value = value;
        m_dirty = true;
    }

    void SetFromThread(const T& value) {
        std::lock_guard<std::mutex> lock(m_threadMutex);
        m_pendingValue = value;
        m_hasPending = true;
    }

    bool IsDirty() const { return m_dirty; }

    void Flush() {
        // Apply thread-safe pending value
        {
            std::lock_guard<std::mutex> lock(m_threadMutex);
            if (m_hasPending) {
                if (!m_dirty) m_oldValueForFlush = m_value;
                m_value = m_pendingValue;
                m_dirty = true;
                m_hasPending = false;
            }
        }

        if (!m_dirty) return;
        m_dirty = false;
        if (!(m_value == m_oldValueForFlush))
            NotifyChanged(m_oldValueForFlush, m_value);
    }

    Property& operator=(const T& value) { Set(value); return *this; }

    T* Ptr() { return &m_value; }
    const T* Ptr() const { return &m_value; }

    Connection OnChanged(ChangedCallback cb) {
        int id = m_nextId++;
        m_callbacks.push_back({id, std::move(cb)});
        int capturedId = id;
        return Connection([this, capturedId]() {
            RemoveCallback(capturedId);
        });
    }

    void RemoveCallback(int id) {
        for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) {
            if (it->id == id) { m_callbacks.erase(it); return; }
        }
    }

    T& MutableRef() { return m_value; }

    void NotifyIfChanged(const T& oldVal) {
        if (!(m_value == oldVal))
            NotifyChanged(oldVal, m_value);
    }

    std::string ToString() const override { return ToStringImpl(); }
    void FromString(const std::string& str) override { FromStringImpl(str); }

private:
    void NotifyChanged(const T& oldVal, const T& newVal) {
        if (m_notifying) return;
        m_notifying = true;
        for (auto& entry : m_callbacks)
            entry.cb(oldVal, newVal);
        m_notifying = false;
    }

    std::string ToStringImpl() const {
        std::ostringstream ss;
        ss << m_value;
        return ss.str();
    }
    void FromStringImpl(const std::string& str) {
        std::istringstream ss(str);
        T val;
        ss >> val;
        Set(val);
    }

    struct CallbackEntry {
        int id;
        ChangedCallback cb;
    };

    T m_value;
    std::vector<CallbackEntry> m_callbacks;
    int m_nextId = 1;
    bool m_notifying = false;

    // Deferred / dirty tracking
    bool m_dirty = false;
    T m_oldValueForFlush{};

    // Thread-safe pending value
    mutable std::mutex m_threadMutex;
    T m_pendingValue{};
    bool m_hasPending = false;
};

inline bool operator==(const glm::vec4& a, const glm::vec4& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
    return os << v.x << " " << v.y << " " << v.z << " " << v.w;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
    return os << v.x << " " << v.y;
}

} // namespace CarHMI::Core

#pragma once

#include "property_base.h"
#include <functional>
#include <vector>
#include <sstream>
#include <glm/glm.hpp>

namespace CarHMI {

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

    Property& operator=(const T& value) { Set(value); return *this; }

    T* Ptr() { return &m_value; }
    const T* Ptr() const { return &m_value; }

    int OnChanged(ChangedCallback cb) {
        int id = m_nextId++;
        m_callbacks.push_back({id, std::move(cb)});
        return id;
    }

    void RemoveCallback(int id) {
        for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) {
            if (it->id == id) { m_callbacks.erase(it); return; }
        }
    }

    // Direct mutation without notification (for animation targets)
    T& MutableRef() { return m_value; }

    // Notify manually after MutableRef changes (e.g. after animation frame)
    void NotifyIfChanged(const T& oldVal) {
        if (!(m_value == oldVal))
            NotifyChanged(oldVal, m_value);
    }

    std::string ToString() const override { return ToStringImpl(); }
    void FromString(const std::string& str) override { FromStringImpl(str); }

private:
    void NotifyChanged(const T& oldVal, const T& newVal) {
        for (auto& entry : m_callbacks)
            entry.cb(oldVal, newVal);
    }

    // Serialization helpers
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
};

// glm::vec4 comparison (needed for Property<glm::vec4>)
inline bool operator==(const glm::vec4& a, const glm::vec4& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
    return os << v.x << " " << v.y << " " << v.z << " " << v.w;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
    return os << v.x << " " << v.y;
}

} // namespace CarHMI

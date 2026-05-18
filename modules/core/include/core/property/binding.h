#pragma once

#include "property.h"
#include <functional>
#include <vector>
#include <memory>

namespace CarHMI::Core {

class BindingBase {
public:
    virtual ~BindingBase() = default;
    virtual void Unbind() = 0;
};

template<typename S, typename T>
class Binding : public BindingBase {
public:
    using Transform = std::function<T(const S&)>;

    Binding(Property<S>& source, Property<T>& target, Transform transform = nullptr)
        : m_source(source), m_target(target), m_transform(std::move(transform)) {
        m_callbackId = m_source.OnChanged([this](const S&, const S& newVal) {
            if (m_transform)
                m_target.Set(m_transform(newVal));
            else if constexpr (std::is_convertible_v<S, T>)
                m_target.Set(static_cast<T>(newVal));
        });
        if (m_transform)
            m_target.Set(m_transform(m_source.Get()));
        else if constexpr (std::is_convertible_v<S, T>)
            m_target.Set(static_cast<T>(m_source.Get()));
    }

    void Unbind() override {
        m_source.RemoveCallback(m_callbackId);
    }

    ~Binding() override { Unbind(); }

private:
    Property<S>& m_source;
    Property<T>& m_target;
    Transform m_transform;
    int m_callbackId = 0;
};

class BindingManager {
public:
    static BindingManager& Get() {
        static BindingManager instance;
        return instance;
    }

    template<typename T>
    void Bind(Property<T>& source, Property<T>& target) {
        m_bindings.push_back(std::make_unique<Binding<T, T>>(source, target));
    }

    template<typename S, typename T>
    void Bind(Property<S>& source, Property<T>& target,
              std::function<T(const S&)> transform) {
        m_bindings.push_back(
            std::make_unique<Binding<S, T>>(source, target, std::move(transform))
        );
    }

    void BindFormat(Property<float>& source, Property<std::string>& target,
                    const std::string& format) {
        Bind<float, std::string>(source, target,
            [format](const float& val) -> std::string {
                char buf[128];
                snprintf(buf, sizeof(buf), format.c_str(), val);
                return std::string(buf);
            }
        );
    }

    void ClearAll() { m_bindings.clear(); }

private:
    BindingManager() = default;
    std::vector<std::unique_ptr<BindingBase>> m_bindings;
};

} // namespace CarHMI::Core

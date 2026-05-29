#pragma once

#include "property.h"
#include <core/connection.h>
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
        m_connection = m_source.OnChanged([this](const S&, const S& newVal) {
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
        m_connection.Disconnect();
    }

    ~Binding() override { Unbind(); }

private:
    Property<S>& m_source;
    Property<T>& m_target;
    Transform m_transform;
    Connection m_connection;
};

template<typename T>
class BidirectionalBinding : public BindingBase {
public:
    BidirectionalBinding(Property<T>& a, Property<T>& b) : m_a(a), m_b(b) {
        m_connA = m_a.OnChanged([this](const T&, const T& newVal) {
            if (!m_updating) {
                m_updating = true;
                m_b.Set(newVal);
                m_updating = false;
            }
        });
        m_connB = m_b.OnChanged([this](const T&, const T& newVal) {
            if (!m_updating) {
                m_updating = true;
                m_a.Set(newVal);
                m_updating = false;
            }
        });
    }

    void Unbind() override {
        m_connA.Disconnect();
        m_connB.Disconnect();
    }

    ~BidirectionalBinding() override { Unbind(); }

private:
    Property<T>& m_a;
    Property<T>& m_b;
    Connection m_connA;
    Connection m_connB;
    bool m_updating = false;
};

class BindingScope {
public:
    BindingScope() = default;
    ~BindingScope() { UnbindAll(); }

    BindingScope(BindingScope&&) noexcept = default;
    BindingScope& operator=(BindingScope&&) noexcept = default;
    BindingScope(const BindingScope&) = delete;
    BindingScope& operator=(const BindingScope&) = delete;

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

    template<typename T>
    void BindBidirectional(Property<T>& a, Property<T>& b) {
        m_bindings.push_back(std::make_unique<BidirectionalBinding<T>>(a, b));
    }

    void UnbindAll() {
        for (auto& b : m_bindings) b->Unbind();
        m_bindings.clear();
    }

private:
    std::vector<std::unique_ptr<BindingBase>> m_bindings;
};

} // namespace CarHMI::Core

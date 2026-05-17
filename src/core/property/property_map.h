#pragma once

#include "property_base.h"
#include "property.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

namespace CarHMI {

class PropertyMap {
public:
    template<typename T>
    Property<T>* Register(const std::string& name, T initial = T{}) {
        auto prop = std::make_unique<Property<T>>(name, initial);
        auto* ptr = prop.get();
        m_properties[name] = std::move(prop);
        m_order.push_back(name);
        return ptr;
    }

    template<typename T>
    Property<T>* Add(const std::string& name, Property<T>* existing) {
        m_refs[name] = existing;
        m_order.push_back(name);
        return existing;
    }

    PropertyBase* Find(const std::string& name) {
        auto it = m_properties.find(name);
        if (it != m_properties.end()) return it->second.get();
        auto rit = m_refs.find(name);
        if (rit != m_refs.end()) return rit->second;
        return nullptr;
    }

    template<typename T>
    Property<T>* Get(const std::string& name) {
        auto* base = Find(name);
        if (!base || base->GetType() != typeid(T)) return nullptr;
        return static_cast<Property<T>*>(base);
    }

    template<typename T>
    bool Set(const std::string& name, const T& value) {
        auto* prop = Get<T>(name);
        if (!prop) return false;
        prop->Set(value);
        return true;
    }

    template<typename T>
    const T* GetValue(const std::string& name) const {
        auto it = m_properties.find(name);
        PropertyBase* base = nullptr;
        if (it != m_properties.end()) base = it->second.get();
        else {
            auto rit = m_refs.find(name);
            if (rit != m_refs.end()) base = rit->second;
        }
        if (!base || base->GetType() != typeid(T)) return nullptr;
        return &static_cast<Property<T>*>(base)->Get();
    }

    const std::vector<std::string>& GetPropertyNames() const { return m_order; }

    void ForEach(std::function<void(const std::string&, PropertyBase&)> fn) {
        for (auto& name : m_order) {
            auto* base = Find(name);
            if (base) fn(name, *base);
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>> m_properties;
    std::unordered_map<std::string, PropertyBase*> m_refs;
    std::vector<std::string> m_order;
};

} // namespace CarHMI

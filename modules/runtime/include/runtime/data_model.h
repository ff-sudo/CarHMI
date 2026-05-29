#pragma once

#include <core/property/property.h>
#include <core/property/property_map.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace CarHMI::Runtime {

/// DataModel: a named collection of observable properties that can be
/// initialized from JSON and bound to Widget properties.
class DataModel {
public:
    explicit DataModel(const std::string& name = "default") : m_name(name) {}

    const std::string& GetName() const { return m_name; }

    /// Initialize properties from a JSON object.
    /// Supported types: float, int (stored as float), bool (stored as float 0/1), string.
    void LoadFromJson(const nlohmann::json& j) {
        for (auto& [key, val] : j.items()) {
            if (val.is_number()) {
                RegisterFloat(key, val.get<float>());
            } else if (val.is_boolean()) {
                RegisterFloat(key, val.get<bool>() ? 1.0f : 0.0f);
            } else if (val.is_string()) {
                RegisterString(key, val.get<std::string>());
            }
        }
    }

    /// Register typed properties manually
    Core::Property<float>* RegisterFloat(const std::string& name, float initial = 0.0f) {
        return m_props.Register<float>(name, initial);
    }

    Core::Property<std::string>* RegisterString(const std::string& name, const std::string& initial = "") {
        return m_props.Register<std::string>(name, initial);
    }

    /// Access properties
    Core::Property<float>* GetFloat(const std::string& name) {
        return m_props.Get<float>(name);
    }

    Core::Property<std::string>* GetString(const std::string& name) {
        return m_props.Get<std::string>(name);
    }

    Core::PropertyBase* Find(const std::string& name) {
        return m_props.Find(name);
    }

    /// Convenience set (returns false if property doesn't exist or type mismatch)
    bool SetFloat(const std::string& name, float value) {
        return m_props.Set<float>(name, value);
    }

    bool SetString(const std::string& name, const std::string& value) {
        return m_props.Set<std::string>(name, value);
    }

    Core::PropertyMap& Props() { return m_props; }
    const Core::PropertyMap& Props() const { return m_props; }

    const std::vector<std::string>& GetPropertyNames() const {
        return m_props.GetPropertyNames();
    }

private:
    std::string m_name;
    Core::PropertyMap m_props;
};

/// Global DataModel registry — scenes can share data models by name.
class DataModelRegistry {
public:
    static DataModelRegistry& Get() {
        static DataModelRegistry instance;
        return instance;
    }

    DataModel* Create(const std::string& name) {
        auto [it, inserted] = m_models.emplace(name, std::make_unique<DataModel>(name));
        return it->second.get();
    }

    DataModel* Find(const std::string& name) {
        auto it = m_models.find(name);
        return (it != m_models.end()) ? it->second.get() : nullptr;
    }

    DataModel* GetOrCreate(const std::string& name) {
        auto* model = Find(name);
        return model ? model : Create(name);
    }

    void Remove(const std::string& name) {
        m_models.erase(name);
    }

    void Clear() { m_models.clear(); }

private:
    std::unordered_map<std::string, std::unique_ptr<DataModel>> m_models;
};

} // namespace CarHMI::Runtime

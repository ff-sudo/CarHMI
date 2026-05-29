#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <spdlog/spdlog.h>

namespace CarHMI::Core {

class ResourceManager {
public:
    ResourceManager() = default;

    [[deprecated("Create ResourceManager instance directly instead")]]
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }

    template<typename T>
    void RegisterLoader(std::function<std::shared_ptr<T>(const std::string&)> loader) {
        m_loaders[std::type_index(typeid(T))] = std::move(loader);
    }

    template<typename T>
    std::shared_ptr<T> Load(const std::string& path) {
        std::string key = MakeKey<T>(path);

        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            if (auto locked = it->second.lock())
                return std::static_pointer_cast<T>(locked);
            m_cache.erase(it);
        }

        auto loaderIt = m_loaders.find(std::type_index(typeid(T)));
        if (loaderIt == m_loaders.end()) {
            spdlog::error("ResourceManager: no loader registered for type {}", typeid(T).name());
            return nullptr;
        }

        auto& loaderAny = loaderIt->second;
        auto& loader = std::any_cast<std::function<std::shared_ptr<T>(const std::string&)>&>(loaderAny);
        auto resource = loader(path);

        if (resource) {
            m_cache[key] = resource;
            spdlog::debug("ResourceManager: loaded '{}'", path);
        }

        return resource;
    }

    template<typename T>
    void Preload(const std::string& path) {
        Load<T>(path);
    }

    bool Contains(const std::string& key) const {
        auto it = m_cache.find(key);
        if (it == m_cache.end()) return false;
        return !it->second.expired();
    }

    void Release(const std::string& key) {
        m_cache.erase(key);
    }

    void CollectGarbage() {
        for (auto it = m_cache.begin(); it != m_cache.end(); ) {
            if (it->second.expired())
                it = m_cache.erase(it);
            else
                ++it;
        }
    }

    void Clear() {
        m_cache.clear();
    }

    size_t CachedCount() const { return m_cache.size(); }

    size_t AliveCount() const {
        size_t count = 0;
        for (auto& [k, v] : m_cache)
            if (!v.expired()) count++;
        return count;
    }

private:
    template<typename T>
    static std::string MakeKey(const std::string& path) {
        return std::string(typeid(T).name()) + "::" + path;
    }

    std::unordered_map<std::string, std::weak_ptr<void>> m_cache;
    std::unordered_map<std::type_index, std::any> m_loaders;
};

} // namespace CarHMI::Core

#pragma once

#include "theme.h"
#include "theme_loader.h"
#include "core/event_bus.h"
#include "core/event.h"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace CarHMI {

class ThemeManager {
public:
    static ThemeManager& Get() {
        static ThemeManager instance;
        return instance;
    }

    void LoadTheme(const std::string& name, const std::string& path) {
        m_themes[name] = ThemeLoader::LoadFromFile(path);
        m_themePaths[name] = path;
        m_lastModified[name] = GetFileTime(path);

        if (m_activeThemeName.empty())
            SetActiveTheme(name);
    }

    void SetActiveTheme(const std::string& name) {
        auto it = m_themes.find(name);
        if (it == m_themes.end()) {
            spdlog::warn("ThemeManager: theme '{}' not found", name);
            return;
        }
        m_activeThemeName = name;
        m_activeTheme = &it->second;
        spdlog::info("ThemeManager: active theme set to '{}'", name);
        EventBus::Get().post(ThemeChangedEvent{m_activeThemeName.c_str()});
    }

    const Theme& GetTheme() const { return *m_activeTheme; }
    const std::string& GetActiveThemeName() const { return m_activeThemeName; }

    void CheckHotReload() {
        for (auto& [name, path] : m_themePaths) {
            auto currentTime = GetFileTime(path);
            if (currentTime > m_lastModified[name]) {
                spdlog::info("ThemeManager: hot-reloading '{}'", name);
                m_themes[name] = ThemeLoader::LoadFromFile(path);
                m_lastModified[name] = currentTime;

                if (name == m_activeThemeName) {
                    m_activeTheme = &m_themes[name];
                    EventBus::Get().post(ThemeChangedEvent{m_activeThemeName.c_str()});
                }
            }
        }
    }

    std::vector<std::string> GetThemeNames() const {
        std::vector<std::string> names;
        for (auto& [name, _] : m_themes)
            names.push_back(name);
        return names;
    }

private:
    ThemeManager() = default;

    static int64_t GetFileTime(const std::string& path) {
        try {
            auto ftime = std::filesystem::last_write_time(path);
            return ftime.time_since_epoch().count();
        } catch (...) {
            return 0;
        }
    }

    std::unordered_map<std::string, Theme> m_themes;
    std::unordered_map<std::string, std::string> m_themePaths;
    std::unordered_map<std::string, int64_t> m_lastModified;

    std::string m_activeThemeName;
    Theme* m_activeTheme = nullptr;
    Theme m_defaultTheme;
};

} // namespace CarHMI

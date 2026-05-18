#pragma once

#include <core/event_bus.h>
#include <core/event.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <fstream>
#include <spdlog/spdlog.h>

namespace CarHMI::GUI {

enum class LayoutDirection { LTR, RTL };

struct LanguageChangedEvent {
    const char* languageCode;
    LayoutDirection direction;
};

class I18n {
public:
    static I18n& Get() {
        static I18n instance;
        return instance;
    }

    void LoadLanguage(const std::string& code, const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) {
            spdlog::warn("I18n: cannot open {}", path);
            return;
        }

        try {
            nlohmann::json root = nlohmann::json::parse(f);
            LanguageData data;
            data.code = code;
            data.name = root.value("name", code);
            data.direction = root.value("direction", "ltr") == "rtl"
                ? LayoutDirection::RTL : LayoutDirection::LTR;

            if (root.contains("strings")) {
                for (auto& [key, val] : root["strings"].items())
                    data.strings[key] = val.get<std::string>();
            }

            m_languages[code] = std::move(data);
            spdlog::info("I18n: loaded language '{}' ({} strings) from {}",
                         code, m_languages[code].strings.size(), path);

            if (m_activeCode.empty())
                SetLanguage(code);

        } catch (const std::exception& e) {
            spdlog::error("I18n: JSON parse error in {}: {}", path, e.what());
        }
    }

    void SetLanguage(const std::string& code) {
        auto it = m_languages.find(code);
        if (it == m_languages.end()) {
            spdlog::warn("I18n: language '{}' not found", code);
            return;
        }
        m_activeCode = code;
        m_activeData = &it->second;
        spdlog::info("I18n: language set to '{}' ({})", code, m_activeData->name);
        Core::EventBus::Get().post(LanguageChangedEvent{m_activeCode.c_str(), m_activeData->direction});
    }

    const std::string& T(const std::string& key) const {
        if (m_activeData) {
            auto it = m_activeData->strings.find(key);
            if (it != m_activeData->strings.end())
                return it->second;
        }
        return key;
    }

    const std::string& GetLanguageCode() const { return m_activeCode; }
    LayoutDirection GetDirection() const {
        return m_activeData ? m_activeData->direction : LayoutDirection::LTR;
    }
    bool IsRTL() const { return GetDirection() == LayoutDirection::RTL; }

    std::vector<std::string> GetLanguageCodes() const {
        std::vector<std::string> codes;
        for (auto& [k, _] : m_languages) codes.push_back(k);
        return codes;
    }

private:
    I18n() = default;

    struct LanguageData {
        std::string code;
        std::string name;
        LayoutDirection direction = LayoutDirection::LTR;
        std::unordered_map<std::string, std::string> strings;
    };

    std::unordered_map<std::string, LanguageData> m_languages;
    std::string m_activeCode;
    LanguageData* m_activeData = nullptr;
};

} // namespace CarHMI::GUI

#include "application.h"
#include <core/scene/scene_manager.h>
#include "dashboard_scene.h"
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>

#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>

using namespace CarHMI;

class GalleryApp : public Gallery::Application {
public:
    GalleryApp() : Application(PAL::WindowConfig{"CarHMI Gallery", 1280, 720}) {}

protected:
    void OnInit() override {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GUI::ThemeManager::Get().LoadTheme("dark", "resources/themes/dark.json");
        GUI::ThemeManager::Get().LoadTheme("light", "resources/themes/light.json");
        GUI::ThemeManager::Get().SetActiveTheme("dark");

        GUI::I18n::Get().LoadLanguage("en", "resources/i18n/en.json");
        GUI::I18n::Get().LoadLanguage("zh", "resources/i18n/zh.json");
        GUI::I18n::Get().LoadLanguage("ar", "resources/i18n/ar.json");
        GUI::I18n::Get().SetLanguage("en");

        GetSceneManager().Push(std::make_unique<Gallery::DashboardScene>());
    }
};

int main(int argc, char* argv[]) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("carhmi.log", true);
    auto logger = std::make_shared<spdlog::logger>("CarHMI",
        spdlog::sinks_init_list{console_sink, file_sink});
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    GalleryApp app;
    app.Run();
    return 0;
}

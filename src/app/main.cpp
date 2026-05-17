#include "core/application.h"
#include "scene/scene_manager.h"
#include "app/dashboard_scene.h"
#include "gui/style/theme_manager.h"
#include "i18n/i18n.h"

#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>

using namespace CarHMI;

class CarHMIApp : public Application {
public:
    CarHMIApp() : Application({"CarHMI - GUI Framework", 1280, 720}) {}

protected:
    void OnInit() override {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ThemeManager::Get().LoadTheme("dark", "resources/themes/dark.json");
        ThemeManager::Get().LoadTheme("light", "resources/themes/light.json");
        ThemeManager::Get().SetActiveTheme("dark");

        I18n::Get().LoadLanguage("en", "resources/i18n/en.json");
        I18n::Get().LoadLanguage("zh", "resources/i18n/zh.json");
        I18n::Get().LoadLanguage("ar", "resources/i18n/ar.json");
        I18n::Get().SetLanguage("en");

        GetSceneManager().Push(std::make_unique<DashboardScene>());
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

    CarHMIApp app;
    app.Run();
    return 0;
}

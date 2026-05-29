#include <runtime/widget_factory.h>
#include <gui/widgets/button.h>
#include <gui/widgets/label.h>
#include <gui/widgets/panel.h>
#include <gui/widgets/image_view.h>
#include <gui/widgets/gauge.h>
#include <gui/widgets/slider.h>
#include <gui/widgets/toggle.h>
#include <gui/widgets/knob.h>
#include <gui/widgets/progress_bar.h>
#include <gui/widgets/scroll_view.h>
#include <gui/widgets/list_widget.h>
#include <gui/widgets/tab_bar.h>
#include <gui/widgets/text_input.h>
#include <gui/widgets/canvas_widget.h>
#include <gui/widgets/dialog.h>
#include <gui/layout/box_layout.h>
#include <spdlog/spdlog.h>

namespace CarHMI::Runtime {

using namespace CarHMI::GUI;
using json = nlohmann::json;

// --- Helpers ---

static glm::vec2 ParseVec2(const json& j, const std::string& key, glm::vec2 fallback = {0, 0}) {
    if (!j.contains(key)) return fallback;
    auto& arr = j[key];
    if (!arr.is_array() || arr.size() < 2) return fallback;
    return {arr[0].get<float>(), arr[1].get<float>()};
}

static glm::vec4 ParseColor(const json& j, const std::string& key, glm::vec4 fallback = {1, 1, 1, 1}) {
    if (!j.contains(key)) return fallback;
    auto& arr = j[key];
    if (!arr.is_array() || arr.size() < 4) return fallback;
    return {arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>(), arr[3].get<float>()};
}

// --- ApplyCommonProps ---

void ApplyCommonProps(GUI::Widget* widget, const json& j) {
    if (j.contains("pos")) widget->SetPos(ParseVec2(j, "pos"));
    if (j.contains("size")) widget->SetSize(ParseVec2(j, "size"));
    if (j.contains("visible")) widget->SetVisible(j["visible"].get<bool>());
    if (j.contains("focusable")) widget->SetFocusable(j["focusable"].get<bool>());
    if (j.contains("fillWidth") && j["fillWidth"].get<bool>()) widget->SetFillWidth();
    if (j.contains("fillHeight") && j["fillHeight"].get<bool>()) widget->SetFillHeight();
    if (j.contains("widthPercent")) widget->SetWidthPercent(j["widthPercent"].get<float>());
    if (j.contains("heightPercent")) widget->SetHeightPercent(j["heightPercent"].get<float>());

    if (j.contains("margin")) {
        auto& m = j["margin"];
        if (m.is_number()) {
            widget->SetMargin(m.get<float>());
        } else if (m.is_array() && m.size() >= 4) {
            widget->SetMargin(Insets(m[0].get<float>(), m[1].get<float>(),
                                     m[2].get<float>(), m[3].get<float>()));
        }
    }
}

// --- WidgetFactory ---

void WidgetFactory::Register(const std::string& typeName, Creator creator) {
    m_creators[typeName] = std::move(creator);
}

GUI::Widget* WidgetFactory::Create(const std::string& typeName, const json& props, int id) {
    auto it = m_creators.find(typeName);
    if (it == m_creators.end()) {
        spdlog::warn("WidgetFactory: unknown type '{}'", typeName);
        return nullptr;
    }
    auto* widget = it->second(props, id);
    if (widget) ApplyCommonProps(widget, props);
    return widget;
}

bool WidgetFactory::HasType(const std::string& typeName) const {
    return m_creators.count(typeName) > 0;
}

void WidgetFactory::RegisterBuiltins() {
    // Button
    Register("Button", [](const json& j, int id) -> Widget* {
        std::string text = j.value("text", "");
        glm::vec2 size = ParseVec2(j, "size", {100, 40});
        return new Button(id, {0, 0}, size, text);
    });

    // Label
    Register("Label", [](const json& j, int id) -> Widget* {
        std::string text = j.value("text", "");
        std::string roleStr = j.value("role", "text");
        Label::Role role = Label::Role::Text;
        if (roleStr == "title") role = Label::Role::Title;
        else if (roleStr == "subtitle") role = Label::Role::Subtitle;
        return new Label(id, {0, 0}, text, role);
    });

    // Panel
    Register("Panel", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {200, 200});
        glm::vec4 bg = ParseColor(j, "bgColor", {-1, -1, -1, -1});
        return new Panel(id, {0, 0}, size, bg);
    });

    // ImageView
    Register("ImageView", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {100, 100});
        return new ImageView(id, {0, 0}, size);
    });

    // Gauge
    Register("Gauge", [](const json& j, int id) -> Widget* {
        float radius = j.value("radius", 80.0f);
        float minVal = j.value("min", 0.0f);
        float maxVal = j.value("max", 240.0f);
        return new Gauge(id, {0, 0}, radius, minVal, maxVal);
    });

    // Slider
    Register("Slider", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {200, 30});
        float minVal = j.value("min", 0.0f);
        float maxVal = j.value("max", 1.0f);
        auto* s = new Slider(id, {0, 0}, size, minVal, maxVal);
        if (j.contains("value")) s->SetValue(j["value"].get<float>());
        return s;
    });

    // Toggle
    Register("Toggle", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {60, 30});
        return new Toggle(id, {0, 0}, size);
    });

    // Knob
    Register("Knob", [](const json& j, int id) -> Widget* {
        float radius = j.value("radius", 40.0f);
        float minVal = j.value("min", 0.0f);
        float maxVal = j.value("max", 100.0f);
        return new Knob(id, {0, 0}, radius, minVal, maxVal);
    });

    // ProgressBar
    Register("ProgressBar", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {200, 25});
        auto* pb = new ProgressBar(id, {0, 0}, size);
        if (j.contains("value")) pb->SetValue(j["value"].get<float>());
        return pb;
    });

    // ScrollView
    Register("ScrollView", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {300, 300});
        glm::vec4 bg = ParseColor(j, "bgColor", {0.12f, 0.12f, 0.16f, 1.0f});
        return new ScrollView(id, {0, 0}, size, bg);
    });

    // ListWidget
    Register("ListWidget", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {300, 200});
        float itemH = j.value("itemHeight", 40.0f);
        return new ListWidget(id, {0, 0}, size, itemH);
    });

    // TabBar
    Register("TabBar", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {400, 50});
        return new TabBar(id, {0, 0}, size);
    });

    // TextInput
    Register("TextInput", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {200, 35});
        std::string ph = j.value("placeholder", "");
        return new TextInput(id, {0, 0}, size, ph);
    });

    // CanvasWidget
    Register("CanvasWidget", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {200, 200});
        return new CanvasWidget(id, {0, 0}, size);
    });

    // BoxLayout / VBoxLayout
    auto boxCreator = [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {400, 400});
        std::string dir = j.value("direction", "vertical");
        float padding = j.value("padding", 10.0f);
        float spacing = j.value("spacing", 8.0f);
        BoxDirection d = (dir == "horizontal") ? BoxDirection::Horizontal : BoxDirection::Vertical;
        return new BoxLayout(id, {0, 0}, size, d, padding, spacing);
    };
    Register("BoxLayout", boxCreator);
    Register("VBoxLayout", boxCreator);

    // HBoxLayout
    Register("HBoxLayout", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {400, 50});
        float padding = j.value("padding", 10.0f);
        float spacing = j.value("spacing", 8.0f);
        return new HBoxLayout(id, {0, 0}, size, padding, spacing);
    });

    // Dialog
    Register("Dialog", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {300, 200});
        std::string title = j.value("title", "Dialog");
        std::string msg = j.value("message", "");
        return new Dialog(id, {0, 0}, size, title, msg);
    });

    // Toast
    Register("Toast", [](const json& j, int id) -> Widget* {
        glm::vec2 size = ParseVec2(j, "size", {300, 60});
        std::string msg = j.value("message", "");
        return new Toast(id, {0, 0}, size, msg);
    });
}

} // namespace CarHMI::Runtime

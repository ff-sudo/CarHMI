#include <gtest/gtest.h>
#include <runtime/widget_factory.h>
#include <gui/widgets/button.h>
#include <gui/widgets/label.h>
#include <gui/widgets/slider.h>
#include <gui/widgets/toggle.h>
#include <gui/widgets/progress_bar.h>
#include <gui/widgets/panel.h>
#include <gui/layout/box_layout.h>
#include <nlohmann/json.hpp>

using namespace CarHMI::Runtime;
using namespace CarHMI::GUI;
using json = nlohmann::json;

class WidgetFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_factory.RegisterBuiltins();
    }
    WidgetFactory m_factory;
};

TEST_F(WidgetFactoryTest, HasBuiltinTypes) {
    EXPECT_TRUE(m_factory.HasType("Button"));
    EXPECT_TRUE(m_factory.HasType("Label"));
    EXPECT_TRUE(m_factory.HasType("Panel"));
    EXPECT_TRUE(m_factory.HasType("Slider"));
    EXPECT_TRUE(m_factory.HasType("Toggle"));
    EXPECT_TRUE(m_factory.HasType("ProgressBar"));
    EXPECT_TRUE(m_factory.HasType("BoxLayout"));
    EXPECT_TRUE(m_factory.HasType("HBoxLayout"));
    EXPECT_TRUE(m_factory.HasType("ImageView"));
    EXPECT_TRUE(m_factory.HasType("ScrollView"));
    EXPECT_TRUE(m_factory.HasType("TextInput"));
    EXPECT_TRUE(m_factory.HasType("Dialog"));
    EXPECT_TRUE(m_factory.HasType("Toast"));
}

TEST_F(WidgetFactoryTest, UnknownTypeReturnsNull) {
    json props = {{"type", "Bogus"}};
    auto* w = m_factory.Create("Bogus", props, 1);
    EXPECT_EQ(w, nullptr);
}

TEST_F(WidgetFactoryTest, CreateButton) {
    json props = {{"text", "Hello"}, {"size", {200, 40}}};
    auto* w = m_factory.Create("Button", props, 10);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetID(), 10);
    EXPECT_EQ(w->GetSize().x, 200.0f);
    EXPECT_EQ(w->GetSize().y, 40.0f);
    delete w;
}

TEST_F(WidgetFactoryTest, CreateLabel) {
    json props = {{"text", "World"}, {"role", "title"}};
    auto* w = m_factory.Create("Label", props, 20);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetID(), 20);
    delete w;
}

TEST_F(WidgetFactoryTest, CreateSliderWithValue) {
    json props = {{"size", {300, 30}}, {"min", 0}, {"max", 100}, {"value", 42}};
    auto* w = m_factory.Create("Slider", props, 30);
    ASSERT_NE(w, nullptr);
    auto* slider = dynamic_cast<Slider*>(w);
    ASSERT_NE(slider, nullptr);
    EXPECT_FLOAT_EQ(slider->GetValue(), 42.0f);
    delete w;
}

TEST_F(WidgetFactoryTest, CreateBoxLayout) {
    json props = {{"size", {400, 300}}, {"direction", "horizontal"}, {"padding", 15}, {"spacing", 10}};
    auto* w = m_factory.Create("BoxLayout", props, 40);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetSize().x, 400.0f);
    delete w;
}

TEST_F(WidgetFactoryTest, ApplyCommonPropsFillWidth) {
    json props = {{"text", "X"}, {"size", {100, 30}}, {"fillWidth", true}, {"margin", {5, 10, 5, 10}}};
    auto* w = m_factory.Create("Button", props, 50);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetSizePolicy().widthMode, SizeMode::Fill);
    EXPECT_FLOAT_EQ(w->GetMargin().top, 5.0f);
    EXPECT_FLOAT_EQ(w->GetMargin().right, 10.0f);
    delete w;
}

TEST_F(WidgetFactoryTest, CustomCreator) {
    WidgetFactory factory;
    factory.Register("Custom", [](const json& j, int id) -> Widget* {
        return new Panel(id, {0, 0}, {50, 50});
    });
    json props = {};
    auto* w = factory.Create("Custom", props, 99);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetID(), 99);
    delete w;
}

TEST_F(WidgetFactoryTest, CreateToggle) {
    json props = {{"size", {80, 40}}};
    auto* w = m_factory.Create("Toggle", props, 60);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetSize().x, 80.0f);
    delete w;
}

TEST_F(WidgetFactoryTest, CreateProgressBar) {
    json props = {{"size", {200, 20}}, {"value", 0.75}};
    auto* w = m_factory.Create("ProgressBar", props, 70);
    ASSERT_NE(w, nullptr);
    auto* pb = dynamic_cast<ProgressBar*>(w);
    ASSERT_NE(pb, nullptr);
    EXPECT_FLOAT_EQ(pb->GetValue(), 0.75f);
    delete w;
}

#include <gtest/gtest.h>
#include <runtime/scene_loader.h>
#include <runtime/widget_factory.h>
#include <gui/widgets/button.h>
#include <gui/widgets/label.h>
#include <gui/widgets/slider.h>
#include <gui/layout/box_layout.h>
#include <nlohmann/json.hpp>

using namespace CarHMI::Runtime;
using namespace CarHMI::GUI;
using json = nlohmann::json;

class SceneLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_factory.RegisterBuiltins();
    }
    WidgetFactory m_factory;
};

TEST_F(SceneLoaderTest, LoadSimpleScene) {
    json scene = {
        {"name", "Test"},
        {"root", {
            {"type", "BoxLayout"},
            {"id", 1},
            {"size", {400, 300}},
            {"children", {
                {{"type", "Label"}, {"id", 2}, {"text", "Hello"}},
                {{"type", "Button"}, {"id", 3}, {"text", "Click"}, {"size", {100, 40}}}
            }}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(loader.GetSceneName(), "Test");
    EXPECT_EQ(root->GetID(), 1);
    EXPECT_EQ(root->GetChildren().size(), 2u);
    EXPECT_EQ(root->GetChildren()[0]->GetID(), 2);
    EXPECT_EQ(root->GetChildren()[1]->GetID(), 3);
    delete root;
}

TEST_F(SceneLoaderTest, UnknownTypeSkipped) {
    json scene = {
        {"name", "Skip"},
        {"root", {
            {"type", "BoxLayout"},
            {"id", 1},
            {"size", {400, 300}},
            {"children", {
                {{"type", "Bogus"}, {"id", 99}},
                {{"type", "Label"}, {"id", 2}, {"text", "OK"}}
            }}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    // Only 1 child (the Label), Bogus skipped
    EXPECT_EQ(root->GetChildren().size(), 1u);
    EXPECT_EQ(root->GetChildren()[0]->GetID(), 2);
    delete root;
}

TEST_F(SceneLoaderTest, EmptyChildren) {
    json scene = {
        {"name", "Empty"},
        {"root", {
            {"type", "BoxLayout"},
            {"id", 1},
            {"size", {400, 300}},
            {"children", json::array()}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetChildren().size(), 0u);
    delete root;
}

TEST_F(SceneLoaderTest, NoChildrenField) {
    json scene = {
        {"name", "NoKids"},
        {"root", {
            {"type", "Button"},
            {"id", 1},
            {"text", "Solo"},
            {"size", {100, 40}}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetChildren().size(), 0u);
    delete root;
}

TEST_F(SceneLoaderTest, NestedLayouts) {
    json scene = {
        {"name", "Nested"},
        {"root", {
            {"type", "BoxLayout"},
            {"id", 1},
            {"size", {400, 400}},
            {"children", {
                {{"type", "HBoxLayout"}, {"id", 10}, {"size", {360, 50}}, {"children", {
                    {{"type", "Button"}, {"id", 11}, {"text", "A"}, {"size", {80, 40}}},
                    {{"type", "Button"}, {"id", 12}, {"text", "B"}, {"size", {80, 40}}}
                }}}
            }}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetChildren().size(), 1u);
    auto* hbox = root->GetChildren()[0];
    EXPECT_EQ(hbox->GetID(), 10);
    EXPECT_EQ(hbox->GetChildren().size(), 2u);
    delete root;
}

TEST_F(SceneLoaderTest, AutoId) {
    json scene = {
        {"name", "AutoId"},
        {"root", {
            {"type", "BoxLayout"},
            {"size", {400, 300}},
            {"children", {
                {{"type", "Label"}, {"text", "NoId"}}
            }}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    // Auto-assigned IDs should be >= 10000
    EXPECT_GE(root->GetID(), 10000);
    EXPECT_GE(root->GetChildren()[0]->GetID(), 10000);
    delete root;
}

TEST_F(SceneLoaderTest, MissingRoot) {
    json scene = {{"name", "Bad"}};
    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    EXPECT_EQ(root, nullptr);
}

TEST_F(SceneLoaderTest, WidgetPropertiesApplied) {
    json scene = {
        {"name", "Props"},
        {"root", {
            {"type", "Slider"},
            {"id", 1},
            {"size", {300, 30}},
            {"min", 0},
            {"max", 200},
            {"value", 100}
        }}
    };

    SceneLoader loader(m_factory);
    auto* root = loader.LoadFromJson(scene);
    ASSERT_NE(root, nullptr);
    auto* slider = dynamic_cast<Slider*>(root);
    ASSERT_NE(slider, nullptr);
    EXPECT_FLOAT_EQ(slider->GetValue(), 100.0f);
    delete root;
}

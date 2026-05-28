#include <gtest/gtest.h>
#include <gui/focus_manager.h>
#include <gui/widget.h>

using namespace CarHMI::GUI;

class FocusManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        FocusManager::Get().Init();
        FocusManager::Get().ClearAll();
    }
    void TearDown() override {
        FocusManager::Get().Shutdown();
    }
};

TEST_F(FocusManagerTest, RegisterWidget) {
    Widget w(1, {0, 0}, {100, 50});
    FocusManager::Get().RegisterWidget(&w);

    auto& chain = FocusManager::Get().GetFocusChain();
    ASSERT_EQ(chain.size(), 1u);
    EXPECT_EQ(chain[0]->GetID(), 1);

    FocusManager::Get().ClearAll(); // clean up before Widget destructor
}

TEST_F(FocusManagerTest, FirstRegisteredWidgetGetsFocus) {
    Widget w(10, {0, 0}, {100, 50});
    FocusManager::Get().RegisterWidget(&w);

    Widget* focused = FocusManager::Get().GetFocused();
    ASSERT_NE(focused, nullptr);
    EXPECT_EQ(focused->GetID(), 10);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, FocusNextWrapsAround) {
    Widget w1(1, {0, 0}, {50, 50});
    Widget w2(2, {0, 0}, {50, 50});

    FocusManager::Get().RegisterWidget(&w1);
    FocusManager::Get().RegisterWidget(&w2);

    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 1);
    FocusManager::Get().FocusNext();
    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 2);
    FocusManager::Get().FocusNext();
    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 1);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, FocusPrevWrapsAround) {
    Widget w1(1, {0, 0}, {50, 50});
    Widget w2(2, {0, 0}, {50, 50});

    FocusManager::Get().RegisterWidget(&w1);
    FocusManager::Get().RegisterWidget(&w2);

    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 1);
    FocusManager::Get().FocusPrev(); // wraps to last
    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 2);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, UnregisterWidget) {
    Widget w1(1, {0, 0}, {50, 50});
    Widget w2(2, {0, 0}, {50, 50});

    FocusManager::Get().RegisterWidget(&w1);
    FocusManager::Get().RegisterWidget(&w2);

    FocusManager::Get().UnregisterWidget(&w1);
    EXPECT_EQ(FocusManager::Get().GetFocusChain().size(), 1u);
    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 2);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, ClearAllRemovesAll) {
    Widget w1(1, {0, 0}, {50, 50});
    Widget w2(2, {0, 0}, {50, 50});

    FocusManager::Get().RegisterWidget(&w1);
    FocusManager::Get().RegisterWidget(&w2);

    FocusManager::Get().ClearAll();
    EXPECT_EQ(FocusManager::Get().GetFocusChain().size(), 0u);
    EXPECT_EQ(FocusManager::Get().GetFocused(), nullptr);
}

TEST_F(FocusManagerTest, IsFocused) {
    Widget w(42, {0, 0}, {50, 50});
    FocusManager::Get().RegisterWidget(&w);

    EXPECT_TRUE(FocusManager::Get().IsFocused(42));
    EXPECT_FALSE(FocusManager::Get().IsFocused(999));

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, GetFocusedId) {
    Widget w(7, {0, 0}, {50, 50});
    FocusManager::Get().RegisterWidget(&w);

    EXPECT_EQ(FocusManager::Get().GetFocusedId(), 7);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, SetFocusIndex) {
    Widget w1(1, {0, 0}, {50, 50});
    Widget w2(2, {0, 0}, {50, 50});

    FocusManager::Get().RegisterWidget(&w1);
    FocusManager::Get().RegisterWidget(&w2);

    FocusManager::Get().SetFocusIndex(1);
    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 2);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, SetFocusIndexOutOfRangeIgnored) {
    Widget w1(1, {0, 0}, {50, 50});
    FocusManager::Get().RegisterWidget(&w1);

    FocusManager::Get().SetFocusIndex(5); // out of range
    EXPECT_EQ(FocusManager::Get().GetFocused()->GetID(), 1); // unchanged

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, ActivateFocused) {
    struct ActivatableWidget : Widget {
        bool activated = false;
        using Widget::Widget;
        void OnFocusActivate() override { activated = true; }
    };

    ActivatableWidget w(1, {0, 0}, {50, 50});
    FocusManager::Get().RegisterWidget(&w);
    FocusManager::Get().ActivateFocused();

    EXPECT_TRUE(w.activated);

    FocusManager::Get().ClearAll();
}

TEST_F(FocusManagerTest, AdjustFocusedValue) {
    struct AdjustableWidget : Widget {
        float value = 0.0f;
        using Widget::Widget;
        void OnFocusAdjust(float delta) override { value += delta; }
    };

    AdjustableWidget w(1, {0, 0}, {50, 50});
    FocusManager::Get().RegisterWidget(&w);
    FocusManager::Get().AdjustFocusedValue(1.5f);

    EXPECT_FLOAT_EQ(w.value, 1.5f);
    FocusManager::Get().AdjustFocusedValue(-0.5f);
    EXPECT_FLOAT_EQ(w.value, 1.0f);

    FocusManager::Get().ClearAll();
}

#include <gtest/gtest.h>
#include <gui/widget.h>

using namespace CarHMI::GUI;

TEST(WidgetTest, Contains) {
    Widget w(1, {10, 20}, {100, 50});

    // Inside
    EXPECT_TRUE(w.Contains({10, 20}));  // top-left corner
    EXPECT_TRUE(w.Contains({110, 70})); // bottom-right corner
    EXPECT_TRUE(w.Contains({60, 45}));  // center

    // Outside
    EXPECT_FALSE(w.Contains({5, 20}));  // left
    EXPECT_FALSE(w.Contains({10, 15})); // above
    EXPECT_FALSE(w.Contains({111, 20})); // right
    EXPECT_FALSE(w.Contains({10, 71})); // below
}

TEST(WidgetTest, GetPos) {
    Widget w(1, {30, 40}, {100, 50});
    EXPECT_FLOAT_EQ(w.GetPos().x, 30.0f);
    EXPECT_FLOAT_EQ(w.GetPos().y, 40.0f);
}

TEST(WidgetTest, SetPos) {
    Widget w(1, {0, 0}, {100, 50});
    w.SetPos({20, 30});
    EXPECT_FLOAT_EQ(w.GetPos().x, 20.0f);
    EXPECT_FLOAT_EQ(w.GetPos().y, 30.0f);
}

TEST(WidgetTest, GetSize) {
    Widget w(1, {0, 0}, {200, 100});
    EXPECT_FLOAT_EQ(w.GetSize().x, 200.0f);
    EXPECT_FLOAT_EQ(w.GetSize().y, 100.0f);
}

TEST(WidgetTest, SetSize) {
    Widget w(1, {0, 0}, {100, 50});
    w.SetSize({300, 150});
    EXPECT_FLOAT_EQ(w.GetSize().x, 300.0f);
    EXPECT_FLOAT_EQ(w.GetSize().y, 150.0f);
}

TEST(WidgetTest, GetID) {
    Widget w(42, {0, 0}, {100, 50});
    EXPECT_EQ(w.GetID(), 42);
}

TEST(WidgetTest, VisibleByDefault) {
    Widget w(1, {0, 0}, {100, 50});
    EXPECT_TRUE(w.IsVisible());
}

TEST(WidgetTest, SetVisible) {
    Widget w(1, {0, 0}, {100, 50});
    w.SetVisible(false);
    EXPECT_FALSE(w.IsVisible());
    w.SetVisible(true);
    EXPECT_TRUE(w.IsVisible());
}

TEST(WidgetTest, Focusable) {
    Widget w(1, {0, 0}, {100, 50});
    EXPECT_FALSE(w.IsFocusable());
    w.SetFocusable(true);
    EXPECT_TRUE(w.IsFocusable());
}

TEST(WidgetTest, GetAbsolutePosNoParent) {
    Widget w(1, {50, 60}, {100, 50});
    glm::vec2 abs = w.GetAbsolutePos();
    EXPECT_FLOAT_EQ(abs.x, 50.0f);
    EXPECT_FLOAT_EQ(abs.y, 60.0f);
}

TEST(WidgetTest, GetAbsolutePosWithParent) {
    auto* parent = new Widget(1, {10, 20}, {200, 200});
    auto* child = new Widget(2, {30, 40}, {50, 50});

    parent->AddChild(child);
    glm::vec2 abs = child->GetAbsolutePos();
    EXPECT_FLOAT_EQ(abs.x, 40.0f); // 10 + 30
    EXPECT_FLOAT_EQ(abs.y, 60.0f); // 20 + 40

    delete parent;
}

TEST(WidgetTest, GetAbsolutePosWithScrollOffset) {
    // Test that scroll offset is accounted for in absolute positioning.
    // We use ScrollView as it inherits Widget and sets m_scrollOffset.
    auto* parent = new Widget(1, {10, 20}, {200, 200});
    auto* child = new Widget(2, {30, 40}, {50, 50});

    parent->AddChild(child);
    // Verify baseline without scroll
    glm::vec2 abs = child->GetAbsolutePos();
    EXPECT_FLOAT_EQ(abs.x, 40.0f); // 10 + 30
    EXPECT_FLOAT_EQ(abs.y, 60.0f); // 20 + 40

    delete parent;
}

TEST(WidgetTest, AddChild) {
    auto* parent = new Widget(1, {0, 0}, {200, 200});
    auto* child = new Widget(2, {0, 0}, {50, 50});

    parent->AddChild(child);
    EXPECT_EQ(parent->GetChildren().size(), 1u);
    EXPECT_EQ(parent->GetChildren()[0]->GetID(), 2);

    delete parent;
}

TEST(WidgetTest, SetWidthPercent) {
    Widget w(1, {0, 0}, {100, 50});
    w.SetWidthPercent(50.0f);

    auto sp = w.GetSizePolicy();
    EXPECT_EQ(sp.widthMode, SizeMode::Percent);
    EXPECT_FLOAT_EQ(sp.widthValue, 50.0f);
    EXPECT_EQ(sp.heightMode, SizeMode::Fixed);
}

TEST(WidgetTest, SetFillWidth) {
    Widget w(1, {0, 0}, {100, 50});
    w.SetFillWidth();

    auto sp = w.GetSizePolicy();
    EXPECT_EQ(sp.widthMode, SizeMode::Fill);
}

TEST(WidgetTest, SetMargin) {
    Widget w(1, {0, 0}, {100, 50});
    w.SetMargin(Insets(5, 10, 15, 20));
    auto m = w.GetMargin();
    EXPECT_FLOAT_EQ(m.top, 5.0f);
    EXPECT_FLOAT_EQ(m.right, 10.0f);
    EXPECT_FLOAT_EQ(m.bottom, 15.0f);
    EXPECT_FLOAT_EQ(m.left, 20.0f);
}

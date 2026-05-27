#include <gtest/gtest.h>
#include <gui/layout/box_layout.h>

using namespace CarHMI::GUI;

// Helper to create a simple Widget for layout testing
static Widget* makeChild(int id, glm::vec2 size) {
    auto* w = new Widget(id, {0, 0}, size);
    return w;
}

class BoxLayoutTest : public ::testing::Test {
protected:
    std::vector<Widget*> owned;

    Widget* child(int id, glm::vec2 size) {
        auto* w = makeChild(id, size);
        owned.push_back(w);
        return w;
    }

    void TearDown() override {
        // Children are owned by layout via AddChild, no manual delete needed
        // unless not added to layout. But BoxLayout destructor deletes children.
    }
};

TEST_F(BoxLayoutTest, VerticalFixedChildren) {
    // Container: 200x400, padding 10, spacing 5
    auto* layout = new BoxLayout(1, {0, 0}, {200, 400}, BoxDirection::Vertical, 10, 5);
    auto* c1 = makeChild(2, {100, 50});
    auto* c2 = makeChild(3, {100, 30});

    layout->AddChild(c1);
    layout->AddChild(c2);

    // c1 should be at (10, 10) [padding]
    EXPECT_FLOAT_EQ(c1->GetPos().x, 10.0f);
    EXPECT_FLOAT_EQ(c1->GetPos().y, 10.0f);

    // c2 should be at (10, 10 + 50 + 0 + 5) = (10, 65)
    EXPECT_FLOAT_EQ(c2->GetPos().x, 10.0f);
    EXPECT_FLOAT_EQ(c2->GetPos().y, 65.0f);

    delete layout;
}

TEST_F(BoxLayoutTest, HorizontalFixedChildren) {
    auto* layout = new BoxLayout(1, {0, 0}, {400, 100}, BoxDirection::Horizontal, 10, 5);
    auto* c1 = makeChild(2, {60, 40});
    auto* c2 = makeChild(3, {80, 40});

    layout->AddChild(c1);
    layout->AddChild(c2);

    EXPECT_FLOAT_EQ(c1->GetPos().x, 10.0f);
    EXPECT_FLOAT_EQ(c1->GetPos().y, 10.0f);

    EXPECT_FLOAT_EQ(c2->GetPos().x, 75.0f); // 10 + 60 + 0 + 5
    EXPECT_FLOAT_EQ(c2->GetPos().y, 10.0f);

    delete layout;
}

TEST_F(BoxLayoutTest, FillWidth) {
    auto* layout = new BoxLayout(1, {0, 0}, {200, 400}, BoxDirection::Vertical, 10, 5);
    auto* c1 = makeChild(2, {0, 50});
    c1->SetFillWidth();

    layout->AddChild(c1);

    // Fill width = availW - margin = 200 - 20 = 180
    EXPECT_FLOAT_EQ(c1->GetSize().x, 180.0f);

    delete layout;
}

TEST_F(BoxLayoutTest, FillHeight) {
    // Container 200x400, padding 10, spacing 5. One fixed child (50h), one fill child
    auto* layout = new BoxLayout(1, {0, 0}, {200, 400}, BoxDirection::Vertical, 10, 5);
    auto* c1 = makeChild(2, {100, 50});
    auto* c2 = makeChild(3, {100, 0});
    c2->SetFillHeight();

    layout->AddChild(c1);
    layout->AddChild(c2);

    // availH = 400 - 20 = 380
    // fixedMain = 50
    // totalSpacing = 5
    // remainAfterFixed = 380 - 50 - 5 = 325
    // fill = 325
    EXPECT_FLOAT_EQ(c2->GetSize().y, 325.0f);

    delete layout;
}

TEST_F(BoxLayoutTest, PercentWidth) {
    auto* layout = new BoxLayout(1, {0, 0}, {200, 100}, BoxDirection::Vertical, 0, 0);
    auto* c1 = makeChild(2, {0, 50});
    c1->SetWidthPercent(50.0f);

    layout->AddChild(c1);

    // 50% of availW (200 - 0) = 100
    EXPECT_FLOAT_EQ(c1->GetSize().x, 100.0f);

    delete layout;
}

TEST_F(BoxLayoutTest, MarginOnChildren) {
    auto* layout = new BoxLayout(1, {0, 0}, {200, 400}, BoxDirection::Vertical, 0, 0);
    auto* c1 = makeChild(2, {100, 50});
    c1->SetMargin(Insets(5, 10, 5, 10));

    layout->AddChild(c1);

    // Position should account for margin: left margin + padding
    EXPECT_FLOAT_EQ(c1->GetPos().x, 10.0f); // padding.left(0) + margin.left(10)
    EXPECT_FLOAT_EQ(c1->GetPos().y, 5.0f);  // padding.top(0) + margin.top(5)

    delete layout;
}

TEST_F(BoxLayoutTest, InsetsPadding) {
    auto* layout = new BoxLayout(1, {0, 0}, {200, 400}, BoxDirection::Vertical, 0, 0);
    layout->SetPadding(Insets(20, 10, 20, 10));
    auto* c1 = makeChild(2, {100, 50});

    layout->AddChild(c1);

    EXPECT_FLOAT_EQ(c1->GetPos().x, 10.0f); // padding.left
    EXPECT_FLOAT_EQ(c1->GetPos().y, 20.0f); // padding.top

    delete layout;
}

#include <gtest/gtest.h>
#include <gui/widget.h>
#include <gui/focus_manager.h>

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

// --- Tree Mutation Safety (deferred AddChild / RemoveChild during Update) ---

TEST(WidgetTest, DeferredAddDuringUpdate) {
    auto* parent = new Widget(1, {0, 0}, {400, 400});
    bool callbackFired = false;

    struct CallbackWidget : Widget {
        std::function<void()> onUpdate;
        using Widget::Widget;
        void Update(UIContext& ctx) override {
            Widget::Update(ctx);
            if (onUpdate) onUpdate();
        }
    };

    auto* child = new CallbackWidget(2, {0, 0}, {100, 50});
    child->onUpdate = [&]() {
        // This callback fires DURING parent->Update() iteration.
        // AddChild would normally invalidate the iterator; deferred mechanism prevents crash.
        auto* newChild = new Widget(3, {0, 0}, {50, 50});
        parent->AddChild(newChild);
        callbackFired = true;
    };

    parent->AddChild(child);
    EXPECT_EQ(parent->GetChildren().size(), 1u);

    // Simulate a frame Update (ctx not used by these widgets)
    UIContext dummyCtx;
    parent->Update(dummyCtx);

    EXPECT_TRUE(callbackFired);
    // The new child was deferred and should be in the tree after Update completes
    EXPECT_EQ(parent->GetChildren().size(), 2u);
    EXPECT_EQ(parent->GetChildren()[1]->GetID(), 3);

    delete parent;
}

TEST(WidgetTest, DeferredRemoveDuringUpdate) {
    auto* parent = new Widget(1, {0, 0}, {400, 400});

    struct CallbackWidget : Widget {
        std::function<void()> onUpdate;
        using Widget::Widget;
        void Update(UIContext& ctx) override {
            Widget::Update(ctx);
            if (onUpdate) onUpdate();
        }
    };

    auto* keeper = new Widget(2, {0, 0}, {50, 50});
    auto* remover = new CallbackWidget(3, {0, 0}, {50, 50});

    Widget* toRemove = new Widget(4, {0, 0}, {50, 50});
    remover->onUpdate = [&]() {
        parent->RemoveChild(toRemove);
    };

    parent->AddChild(keeper);
    parent->AddChild(remover);
    parent->AddChild(toRemove);
    EXPECT_EQ(parent->GetChildren().size(), 3u);

    UIContext dummyCtx;
    parent->Update(dummyCtx);

    EXPECT_EQ(parent->GetChildren().size(), 2u);
    // toRemove should be gone, keeper and remover remain
    EXPECT_EQ(parent->GetChildren()[0]->GetID(), 2);
    EXPECT_EQ(parent->GetChildren()[1]->GetID(), 3);
    // toRemove is NOT deleted by RemoveChild — caller owns the pointer
    EXPECT_EQ(toRemove->GetID(), 4);
    delete toRemove;
    delete parent;
}

TEST(WidgetTest, RemoveChildUnregistersFocus) {
    // FocusManager must be inited before use
    FocusManager::Get().Init();

    auto* parent = new Widget(1, {0, 0}, {400, 400});
    auto* child = new Widget(2, {0, 0}, {50, 50});
    child->SetFocusable(true);

    parent->AddChild(child);
    FocusManager::Get().RegisterWidget(child);
    EXPECT_EQ(FocusManager::Get().GetFocusChain().size(), 1u);

    parent->RemoveChild(child);
    // After RemoveChild, child should no longer be in the focus chain
    EXPECT_EQ(FocusManager::Get().GetFocusChain().size(), 0u);

    delete child;
    delete parent;
    FocusManager::Get().Shutdown();
}

TEST(WidgetTest, NestedDeferredMutations) {
    // Grandparent → parent → child: leaf callback modifies grandparent's children
    auto* grandparent = new Widget(10, {0, 0}, {800, 800});

    auto* middle = new Widget(20, {0, 0}, {400, 400});

    struct CallbackWidget : Widget {
        std::function<void()> onUpdate;
        using Widget::Widget;
        void Update(UIContext& ctx) override {
            Widget::Update(ctx);
            if (onUpdate) onUpdate();
        }
    };
    auto* leaf = new CallbackWidget(30, {0, 0}, {100, 50});

    Widget* newSibling = new Widget(40, {0, 0}, {50, 50});

    leaf->onUpdate = [&]() {
        // Cross-level modification: leaf adds to grandparent during traversal
        grandparent->AddChild(newSibling);
    };

    grandparent->AddChild(middle);
    middle->AddChild(leaf);
    EXPECT_EQ(grandparent->GetChildren().size(), 1u); // just middle

    UIContext dummyCtx;
    grandparent->Update(dummyCtx);

    // After Update completes, grandparent should have middle + newSibling
    EXPECT_EQ(grandparent->GetChildren().size(), 2u);
    EXPECT_EQ(grandparent->GetChildren()[1]->GetID(), 40);

    delete grandparent;
}

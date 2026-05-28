#include <gtest/gtest.h>
#include <gui/widgets/dialog.h>
#include <gui/widgets/label.h>

using namespace CarHMI::GUI;

TEST(DialogTest, Construction) {
    Dialog dlg(1, {50, 50}, {300, 200}, "Alert", "Are you sure?");
    EXPECT_EQ(dlg.GetID(), 1);
    EXPECT_TRUE(dlg.IsVisible());

    // Dialog should have children (title label, message label, button row)
    EXPECT_GE(dlg.GetChildren().size(), 2u);
}

TEST(DialogTest, AddButton) {
    int clicked = 0;
    Dialog dlg(1, {50, 50}, {300, 200}, "Dialog");

    dlg.AddButton("OK", [&clicked]() { clicked = 1; });

    // Button row should have 1 child
    auto& children = dlg.GetChildren();
    ASSERT_GE(children.size(), 3u);

    // The last child is the button row (HBoxLayout)
    auto* btnRow = children.back();
    EXPECT_GE(btnRow->GetChildren().size(), 1u);
}

TEST(DialogTest, MultipleButtons) {
    Dialog dlg(1, {50, 50}, {400, 250}, "Confirm");
    dlg.AddButton("Cancel", []() {});
    dlg.AddButton("Yes", []() {});
    dlg.AddButton("No", []() {});

    auto& children = dlg.GetChildren();
    ASSERT_GE(children.size(), 3u);

    auto* btnRow = children.back();
    EXPECT_EQ(btnRow->GetChildren().size(), 3u);
}

TEST(DialogTest, SetTitleAndMessage) {
    Dialog dlg(1, {0, 0}, {300, 200}, "Old Title", "Old Message");
    dlg.SetTitle("New Title");
    dlg.SetMessage("New Message");

    // First child is the title label
    auto* titleLabel = dynamic_cast<Label*>(dlg.GetChildren()[0]);
    ASSERT_NE(titleLabel, nullptr);
    EXPECT_EQ(titleLabel->GetText(), "New Title");
}

TEST(DialogTest, SetCornerRadius) {
    Dialog dlg(1, {0, 0}, {300, 200});
    dlg.SetCornerRadius(8.0f);
    // Visual-only, verify no crash
}

TEST(DialogTest, TitleI18nKey) {
    Dialog dlg(1, {0, 0}, {300, 200}, "Title");
    dlg.SetTitleI18n("dialog.title");
    dlg.SetMessageI18n("dialog.message");
    // Verify no crash
}

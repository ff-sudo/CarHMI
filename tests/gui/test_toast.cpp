#include <gtest/gtest.h>
#include <gui/widgets/dialog.h>

using namespace CarHMI::GUI;

TEST(ToastTest, Construction) {
    Toast t(1, {100, 20}, {200, 40}, "Operation completed");
    EXPECT_EQ(t.GetID(), 1);
    EXPECT_FALSE(t.IsVisible()); // hidden until Show()
    t.Show();
    EXPECT_TRUE(t.IsVisible());  // visible after Show()
}

TEST(ToastTest, SetMessage) {
    Toast t(1, {100, 20}, {200, 40}, "Loading...");
    t.SetMessage("Done");
    // Verify no crash
}

TEST(ToastTest, SetDuration) {
    Toast t(1, {100, 20}, {200, 40}, "Message");
    t.SetDuration(5.0f);
    // Verify no crash
}

TEST(ToastTest, SetType) {
    Toast t(1, {100, 20}, {200, 40}, "Success!");
    t.SetType(ToastType::Success);
    // Verify no crash

    Toast t2(2, {100, 20}, {200, 40}, "Error!");
    t2.SetType(ToastType::Error);
}

TEST(ToastTest, ShowAndHide) {
    Toast t(1, {100, 20}, {200, 40}, "Test message");
    t.Show();
    // Should be visible after show
    EXPECT_TRUE(t.IsVisible());
    t.Hide();
    // Hide will animate opacity to 0 then set visible=false
}

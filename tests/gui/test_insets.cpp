#include <gtest/gtest.h>
#include <gui/layout/insets.h>

using namespace CarHMI::GUI;

TEST(InsetsTest, DefaultConstructor) {
    Insets i;
    EXPECT_FLOAT_EQ(i.top, 0.0f);
    EXPECT_FLOAT_EQ(i.right, 0.0f);
    EXPECT_FLOAT_EQ(i.bottom, 0.0f);
    EXPECT_FLOAT_EQ(i.left, 0.0f);
}

TEST(InsetsTest, UniformConstructor) {
    Insets i(10.0f);
    EXPECT_FLOAT_EQ(i.top, 10.0f);
    EXPECT_FLOAT_EQ(i.right, 10.0f);
    EXPECT_FLOAT_EQ(i.bottom, 10.0f);
    EXPECT_FLOAT_EQ(i.left, 10.0f);
}

TEST(InsetsTest, TwoValueConstructor) {
    Insets i(5.0f, 10.0f);
    EXPECT_FLOAT_EQ(i.top, 5.0f);
    EXPECT_FLOAT_EQ(i.bottom, 5.0f);
    EXPECT_FLOAT_EQ(i.right, 10.0f);
    EXPECT_FLOAT_EQ(i.left, 10.0f);
}

TEST(InsetsTest, FourValueConstructor) {
    Insets i(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(i.top, 1.0f);
    EXPECT_FLOAT_EQ(i.right, 2.0f);
    EXPECT_FLOAT_EQ(i.bottom, 3.0f);
    EXPECT_FLOAT_EQ(i.left, 4.0f);
}

TEST(InsetsTest, HorizontalVertical) {
    Insets i(10.0f, 20.0f, 30.0f, 40.0f);
    EXPECT_FLOAT_EQ(i.horizontal(), 60.0f); // right + left
    EXPECT_FLOAT_EQ(i.vertical(), 40.0f);   // top + bottom
}

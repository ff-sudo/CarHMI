#include <gtest/gtest.h>
#include <gui/layout/size_policy.h>

using namespace CarHMI::GUI;

TEST(SizePolicyTest, DefaultIsFixed) {
    SizePolicy sp;
    EXPECT_EQ(sp.widthMode, SizeMode::Fixed);
    EXPECT_EQ(sp.heightMode, SizeMode::Fixed);
    EXPECT_FLOAT_EQ(sp.widthValue, 0.0f);
    EXPECT_FLOAT_EQ(sp.heightValue, 0.0f);
}

TEST(SizePolicyTest, EnumValues) {
    EXPECT_NE(SizeMode::Fixed, SizeMode::Percent);
    EXPECT_NE(SizeMode::Percent, SizeMode::Fill);
    EXPECT_NE(SizeMode::Fixed, SizeMode::Fill);
}

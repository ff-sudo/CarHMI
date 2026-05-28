#include <gtest/gtest.h>
#include <gui/widgets/image_view.h>

using namespace CarHMI::GUI;

TEST(ImageViewTest, DefaultScaleMode) {
    ImageView iv(1, {0, 0}, {100, 100});
    EXPECT_EQ(iv.GetScaleMode(), ImageView::ScaleMode::Fill);
}

TEST(ImageViewTest, SetScaleMode) {
    ImageView iv(1, {0, 0}, {100, 100});
    iv.SetScaleMode(ImageView::ScaleMode::Fit);
    EXPECT_EQ(iv.GetScaleMode(), ImageView::ScaleMode::Fit);
}

TEST(ImageViewTest, NoTextureReturnsNull) {
    ImageView iv(1, {0, 0}, {100, 100});
    EXPECT_EQ(iv.GetTexture(), nullptr);
}

TEST(ImageViewTest, SetAndGetTexture) {
    ImageView iv(1, {0, 0}, {100, 100});
    CarHMI::RHI::Texture2D tex;
    iv.SetTexture(&tex);
    EXPECT_EQ(iv.GetTexture(), &tex);
}

TEST(ImageViewTest, SetTint) {
    ImageView iv(1, {0, 0}, {100, 100});
    iv.SetTint({0.5f, 0.5f, 0.5f, 0.8f});
}

TEST(ImageViewTest, SetBgColor) {
    ImageView iv(1, {0, 0}, {100, 100});
    iv.SetBgColor({0.1f, 0.2f, 0.3f, 1.0f});
}

TEST(ImageViewTest, BasicProperties) {
    ImageView iv(42, {10, 20}, {200, 150});
    EXPECT_EQ(iv.GetID(), 42);
    EXPECT_FLOAT_EQ(iv.GetPos().x, 10.0f);
    EXPECT_FLOAT_EQ(iv.GetPos().y, 20.0f);
    EXPECT_FLOAT_EQ(iv.GetSize().x, 200.0f);
    EXPECT_FLOAT_EQ(iv.GetSize().y, 150.0f);
    EXPECT_TRUE(iv.IsVisible());
}

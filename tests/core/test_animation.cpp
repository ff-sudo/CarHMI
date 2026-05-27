#include <gtest/gtest.h>
#include <core/animation/easing.h>
#include <core/animation/tween.h>
#include <core/animation/animation_manager.h>

using namespace CarHMI::Core;

// --- Easing Tests ---

TEST(EasingTest, LinearBoundaries) {
    EXPECT_FLOAT_EQ(Easing::Linear(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::Linear(1.0f), 1.0f);
    EXPECT_FLOAT_EQ(Easing::Linear(0.5f), 0.5f);
}

TEST(EasingTest, QuadBoundaries) {
    EXPECT_FLOAT_EQ(Easing::InQuad(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::InQuad(1.0f), 1.0f);
    EXPECT_FLOAT_EQ(Easing::OutQuad(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::OutQuad(1.0f), 1.0f);
}

TEST(EasingTest, CubicBoundaries) {
    EXPECT_FLOAT_EQ(Easing::InCubic(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::InCubic(1.0f), 1.0f);
    EXPECT_FLOAT_EQ(Easing::OutCubic(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::OutCubic(1.0f), 1.0f);
}

TEST(EasingTest, ExpoBoundaries) {
    EXPECT_FLOAT_EQ(Easing::InExpo(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::InExpo(1.0f), 1.0f);
    EXPECT_FLOAT_EQ(Easing::OutExpo(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::OutExpo(1.0f), 1.0f);
}

TEST(EasingTest, BounceBoundaries) {
    EXPECT_NEAR(Easing::OutBounce(0.0f), 0.0f, 1e-5f);
    EXPECT_NEAR(Easing::OutBounce(1.0f), 1.0f, 1e-5f);
    EXPECT_NEAR(Easing::InBounce(0.0f), 0.0f, 1e-5f);
    EXPECT_NEAR(Easing::InBounce(1.0f), 1.0f, 1e-5f);
}

// --- Tween Tests ---

TEST(TweenTest, LinearInterpolation) {
    float target = 0.0f;
    Tween<float> tween(&target, 0.0f, 100.0f, 1.0f, Easing::Linear);

    tween.Update(0.5f);
    EXPECT_FLOAT_EQ(target, 50.0f);
    EXPECT_FALSE(tween.IsFinished());

    tween.Update(0.5f);
    EXPECT_FLOAT_EQ(target, 100.0f);
    EXPECT_TRUE(tween.IsFinished());
}

TEST(TweenTest, Reset) {
    float target = 0.0f;
    Tween<float> tween(&target, 0.0f, 100.0f, 1.0f);

    tween.Update(1.0f);
    EXPECT_TRUE(tween.IsFinished());

    tween.Reset();
    EXPECT_FALSE(tween.IsFinished());
    target = 0.0f;
    tween.Update(0.5f);
    EXPECT_FLOAT_EQ(target, 50.0f);
}

TEST(TweenTest, OnComplete) {
    float target = 0.0f;
    bool completed = false;
    Tween<float> tween(&target, 0.0f, 1.0f, 1.0f);
    tween.SetOnComplete([&completed]() { completed = true; });

    tween.Update(0.5f);
    EXPECT_FALSE(completed);
    tween.Update(0.6f);
    EXPECT_TRUE(completed);
}

// --- AnimationManager Tests ---

class AnimationManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        AnimationManager::Get().Clear();
    }
    void TearDown() override {
        AnimationManager::Get().Clear();
    }
};

TEST_F(AnimationManagerTest, TweenToAndUpdate) {
    float val = 0.0f;
    AnimationManager::Get().TweenTo(&val, 100.0f, 1.0f);
    EXPECT_EQ(AnimationManager::Get().ActiveCount(), 1);

    AnimationManager::Get().Update(1.0f);
    EXPECT_FLOAT_EQ(val, 100.0f);
    EXPECT_EQ(AnimationManager::Get().ActiveCount(), 0); // finished, removed
}

TEST_F(AnimationManagerTest, Clear) {
    float val = 0.0f;
    AnimationManager::Get().TweenTo(&val, 100.0f, 1.0f);
    AnimationManager::Get().TweenTo(&val, 200.0f, 2.0f);
    EXPECT_EQ(AnimationManager::Get().ActiveCount(), 2);
    AnimationManager::Get().Clear();
    EXPECT_EQ(AnimationManager::Get().ActiveCount(), 0);
}

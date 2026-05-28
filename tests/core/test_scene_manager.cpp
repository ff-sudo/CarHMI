#include <gtest/gtest.h>
#include <core/scene/scene_manager.h>
#include <core/animation/animation_manager.h>

using namespace CarHMI::Core;

// Spy scene that records lifecycle calls
class SpyScene : public Scene {
public:
    std::vector<std::string> calls;
    std::string name;

    explicit SpyScene(const std::string& n) : name(n) {}

    void OnEnter() override { calls.push_back("enter:" + name); }
    void OnExit() override { calls.push_back("exit:" + name); }
    void OnPause() override { calls.push_back("pause:" + name); }
    void OnResume() override { calls.push_back("resume:" + name); }
    const char* GetName() const override { return name.c_str(); }
};

class SceneManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        AnimationManager::Get().Clear();
    }
    void TearDown() override {
        AnimationManager::Get().Clear();
    }
};

TEST_F(SceneManagerTest, PushCallsOnEnter) {
    SceneManager sm;
    auto scene = std::make_unique<SpyScene>("A");

    sm.Push(std::move(scene), TransitionType::None);

    auto* current = dynamic_cast<SpyScene*>(sm.Current());
    ASSERT_NE(current, nullptr);
    EXPECT_EQ(current->calls.size(), 1u);
    EXPECT_EQ(current->calls[0], "enter:A");
    EXPECT_EQ(sm.StackSize(), 1);
}

TEST_F(SceneManagerTest, PushPausesPrevious) {
    SceneManager sm;
    auto a = std::make_unique<SpyScene>("A");
    auto* aPtr = a.get();
    sm.Push(std::move(a), TransitionType::None);

    auto b = std::make_unique<SpyScene>("B");
    sm.Push(std::move(b), TransitionType::None);

    // A should be paused
    ASSERT_GE(aPtr->calls.size(), 2u);
    EXPECT_EQ(aPtr->calls[1], "pause:A");
    EXPECT_EQ(sm.StackSize(), 2);
}

TEST_F(SceneManagerTest, PopCallsOnExitAndResumePrevious) {
    SceneManager sm;
    auto a = std::make_unique<SpyScene>("A");
    auto* aPtr = a.get();
    sm.Push(std::move(a), TransitionType::None);

    auto b = std::make_unique<SpyScene>("B");
    sm.Push(std::move(b), TransitionType::None);

    // Save A's call count before Pop (B is current)
    size_t aCallsBefore = aPtr->calls.size(); // should be 2: enter + pause

    sm.Pop(TransitionType::None);
    sm.Update(0.0f); // triggers ProcessPendingPop

    // A should be resumed (3rd call: resume)
    ASSERT_EQ(aPtr->calls.size(), aCallsBefore + 1);
    EXPECT_EQ(aPtr->calls.back(), "resume:A");
    EXPECT_EQ(sm.StackSize(), 1);
}

TEST_F(SceneManagerTest, PopEmptyStackIsSafe) {
    SceneManager sm;
    sm.Pop(TransitionType::None);
    sm.Update(0.0f);
    EXPECT_EQ(sm.StackSize(), 0);
}

TEST_F(SceneManagerTest, ReplaceSwapsCurrent) {
    SceneManager sm;
    auto a = std::make_unique<SpyScene>("A");
    sm.Push(std::move(a), TransitionType::None);

    auto b = std::make_unique<SpyScene>("B");
    sm.Replace(std::move(b), TransitionType::None);

    EXPECT_EQ(sm.StackSize(), 1);

    auto* current = dynamic_cast<SpyScene*>(sm.Current());
    ASSERT_NE(current, nullptr);
    // B was entered
    ASSERT_GE(current->calls.size(), 1u);
    EXPECT_EQ(current->calls[0], "enter:B");
}

TEST_F(SceneManagerTest, CurrentNullWhenEmpty) {
    SceneManager sm;
    EXPECT_EQ(sm.Current(), nullptr);
}

TEST_F(SceneManagerTest, DeferredPop) {
    SceneManager sm;
    auto a = std::make_unique<SpyScene>("A");
    sm.Push(std::move(a), TransitionType::None);

    // Pop defers, stack still has 1
    sm.Pop(TransitionType::None);
    EXPECT_EQ(sm.StackSize(), 1);

    // ProcessPendingPop is called at end of Update
    sm.Update(0.0f);
    EXPECT_EQ(sm.StackSize(), 0);
}

TEST_F(SceneManagerTest, TransitionState) {
    SceneManager sm;
    auto a = std::make_unique<SpyScene>("A");
    sm.Push(std::move(a), TransitionType::SlideLeft);

    EXPECT_TRUE(sm.IsTransitioning());

    // Complete transition
    AnimationManager::Get().Update(0.5f);
    EXPECT_FALSE(sm.IsTransitioning());
}

TEST_F(SceneManagerTest, NoneTransitionSkipsAnimation) {
    SceneManager sm;
    auto a = std::make_unique<SpyScene>("A");
    sm.Push(std::move(a), TransitionType::None);
    EXPECT_FALSE(sm.IsTransitioning());
}

TEST_F(SceneManagerTest, UpdateCallsOnUpdateAndOnImGui) {
    SceneManager sm;

    struct UpdateSpyScene : Scene {
        bool updated = false, imguiCalled = false;
        void OnUpdate(float) override { updated = true; }
        void OnImGui() override { imguiCalled = true; }
    };

    auto* spy = new UpdateSpyScene();
    sm.Push(std::unique_ptr<Scene>(spy), TransitionType::None);
    sm.Update(0.016f);

    EXPECT_TRUE(spy->updated);
    EXPECT_TRUE(spy->imguiCalled);
}

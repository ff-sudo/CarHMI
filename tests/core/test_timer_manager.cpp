#include <gtest/gtest.h>
#include <core/timer/timer_manager.h>

using namespace CarHMI::Core;

class TimerManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        TimerManager::Get().ClearAll();
    }
    void TearDown() override {
        TimerManager::Get().ClearAll();
    }
};

TEST_F(TimerManagerTest, SetTimeoutFires) {
    int count = 0;
    auto conn = TimerManager::Get().SetTimeout([&count]() { count++; }, 100);
    EXPECT_EQ(TimerManager::Get().ActiveCount(), 1);

    TimerManager::Get().Update(0.05f); // 50ms
    EXPECT_EQ(count, 0);

    TimerManager::Get().Update(0.06f); // 60ms more = 110ms total
    EXPECT_EQ(count, 1);
    EXPECT_EQ(TimerManager::Get().ActiveCount(), 0); // one-shot removed
}

TEST_F(TimerManagerTest, SetIntervalRepeats) {
    int count = 0;
    auto conn = TimerManager::Get().SetInterval([&count]() { count++; }, 100);

    TimerManager::Get().Update(0.11f); // 110ms
    EXPECT_EQ(count, 1);

    TimerManager::Get().Update(0.10f); // 100ms more = 210ms total (10ms carried over from first)
    EXPECT_EQ(count, 2);

    EXPECT_EQ(TimerManager::Get().ActiveCount(), 1);
}

TEST_F(TimerManagerTest, CancelViaConnection) {
    int count = 0;
    {
        auto conn = TimerManager::Get().SetInterval([&count]() { count++; }, 100);
    } // conn destroyed, timer cancelled

    TimerManager::Get().Update(0.2f);
    EXPECT_EQ(count, 0);
}

TEST_F(TimerManagerTest, ClearAll) {
    auto c1 = TimerManager::Get().SetTimeout([]() {}, 100);
    auto c2 = TimerManager::Get().SetInterval([]() {}, 200);
    EXPECT_EQ(TimerManager::Get().ActiveCount(), 2);

    TimerManager::Get().ClearAll();
    EXPECT_EQ(TimerManager::Get().ActiveCount(), 0);
}

TEST_F(TimerManagerTest, PrecisionAccumulation) {
    int count = 0;
    auto conn = TimerManager::Get().SetTimeout([&count]() { count++; }, 100);

    // Three 40ms updates = 120ms total, should fire after third
    TimerManager::Get().Update(0.04f);
    EXPECT_EQ(count, 0);
    TimerManager::Get().Update(0.04f);
    EXPECT_EQ(count, 0);
    TimerManager::Get().Update(0.04f);
    EXPECT_EQ(count, 1);
}

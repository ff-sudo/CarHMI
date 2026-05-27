#include <gtest/gtest.h>
#include <core/event_bus.h>

using namespace CarHMI::Core;

struct TestEventA { int value; };
struct TestEventB { std::string msg; };

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventBus::Get().FlushDeferred();
    }
};

TEST_F(EventBusTest, SubscribeAndPost) {
    int received = 0;
    auto conn = EventBus::Get().Subscribe<TestEventA>([&received](const TestEventA& e) {
        received = e.value;
    });
    EventBus::Get().Post(TestEventA{42});
    EXPECT_EQ(received, 42);
}

TEST_F(EventBusTest, MultipleSubscribers) {
    int count = 0;
    auto c1 = EventBus::Get().Subscribe<TestEventA>([&count](const TestEventA&) { count++; });
    auto c2 = EventBus::Get().Subscribe<TestEventA>([&count](const TestEventA&) { count++; });
    EventBus::Get().Post(TestEventA{1});
    EXPECT_EQ(count, 2);
}

TEST_F(EventBusTest, TypeIsolation) {
    bool aReceived = false, bReceived = false;
    auto c1 = EventBus::Get().Subscribe<TestEventA>([&aReceived](const TestEventA&) { aReceived = true; });
    auto c2 = EventBus::Get().Subscribe<TestEventB>([&bReceived](const TestEventB&) { bReceived = true; });
    EventBus::Get().Post(TestEventA{0});
    EXPECT_TRUE(aReceived);
    EXPECT_FALSE(bReceived);
}

TEST_F(EventBusTest, UnsubscribeViaConnection) {
    int count = 0;
    {
        auto conn = EventBus::Get().Subscribe<TestEventA>([&count](const TestEventA&) { count++; });
        EventBus::Get().Post(TestEventA{0});
        EXPECT_EQ(count, 1);
    } // conn destroyed, unsubscribed
    EventBus::Get().Post(TestEventA{0});
    EXPECT_EQ(count, 1);
}

TEST_F(EventBusTest, PostDeferredAndFlush) {
    int received = 0;
    auto conn = EventBus::Get().Subscribe<TestEventA>([&received](const TestEventA& e) {
        received = e.value;
    });
    EventBus::Get().PostDeferred(TestEventA{99});
    EXPECT_EQ(received, 0); // not yet delivered
    EventBus::Get().FlushDeferred();
    EXPECT_EQ(received, 99);
}

TEST_F(EventBusTest, PostToNoSubscribers) {
    // Should not crash
    EventBus::Get().Post(TestEventB{"hello"});
}

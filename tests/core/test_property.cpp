#include <gtest/gtest.h>
#include <core/property/property.h>

using namespace CarHMI::Core;

TEST(PropertyTest, GetAndSet) {
    Property<int> p("test", 10);
    EXPECT_EQ(p.Get(), 10);
    p.Set(20);
    EXPECT_EQ(p.Get(), 20);
}

TEST(PropertyTest, OnChangedFires) {
    Property<int> p("test", 0);
    int oldVal = -1, newVal = -1;
    auto conn = p.OnChanged([&](const int& o, const int& n) {
        oldVal = o;
        newVal = n;
    });
    p.Set(42);
    EXPECT_EQ(oldVal, 0);
    EXPECT_EQ(newVal, 42);
}

TEST(PropertyTest, EqualSkipsNotification) {
    Property<int> p("test", 5);
    int count = 0;
    auto conn = p.OnChanged([&count](const int&, const int&) { count++; });
    p.Set(5); // same value
    EXPECT_EQ(count, 0);
    p.Set(6);
    EXPECT_EQ(count, 1);
}

TEST(PropertyTest, DisconnectStopsCallbacks) {
    Property<int> p("test", 0);
    int count = 0;
    {
        auto conn = p.OnChanged([&count](const int&, const int&) { count++; });
        p.Set(1);
        EXPECT_EQ(count, 1);
    }
    p.Set(2);
    EXPECT_EQ(count, 1);
}

TEST(PropertyTest, SetDeferredAndFlush) {
    Property<int> p("test", 0);
    int count = 0;
    auto conn = p.OnChanged([&count](const int&, const int&) { count++; });
    p.SetDeferred(10);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(p.Get(), 10); // value is updated immediately
    EXPECT_TRUE(p.IsDirty());
    p.Flush();
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(p.IsDirty());
}

TEST(PropertyTest, SetFromThreadAndFlush) {
    Property<int> p("test", 0);
    int count = 0;
    auto conn = p.OnChanged([&count](const int&, const int&) { count++; });
    p.SetFromThread(100);
    EXPECT_EQ(count, 0);
    p.Flush();
    EXPECT_EQ(p.Get(), 100);
    EXPECT_EQ(count, 1);
}

TEST(PropertyTest, ReentrancyGuard) {
    Property<int> p("test", 0);
    int count = 0;
    auto conn = p.OnChanged([&](const int&, const int&) {
        count++;
        p.Set(99); // re-entrant set should be suppressed
    });
    p.Set(1);
    EXPECT_EQ(count, 1); // only once, re-entrant suppressed
}

TEST(PropertyTest, MultipleCallbacks) {
    Property<int> p("test", 0);
    int c1 = 0, c2 = 0;
    auto conn1 = p.OnChanged([&c1](const int&, const int&) { c1++; });
    auto conn2 = p.OnChanged([&c2](const int&, const int&) { c2++; });
    p.Set(1);
    EXPECT_EQ(c1, 1);
    EXPECT_EQ(c2, 1);
}

TEST(PropertyTest, OperatorAssign) {
    Property<float> p("test", 0.0f);
    p = 3.14f;
    EXPECT_FLOAT_EQ(p.Get(), 3.14f);
}

TEST(PropertyTest, ToStringFromString) {
    Property<int> p("test", 42);
    EXPECT_EQ(p.ToString(), "42");
    p.FromString("100");
    EXPECT_EQ(p.Get(), 100);
}

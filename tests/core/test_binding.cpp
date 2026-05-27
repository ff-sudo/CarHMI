#include <gtest/gtest.h>
#include <core/property/binding.h>

using namespace CarHMI::Core;

TEST(BindingTest, OneWaySync) {
    Property<float> source("source", 10.0f);
    Property<float> target("target", 0.0f);

    Binding<float, float> binding(source, target);
    EXPECT_FLOAT_EQ(target.Get(), 10.0f); // initial sync

    source.Set(20.0f);
    EXPECT_FLOAT_EQ(target.Get(), 20.0f);
}

TEST(BindingTest, OneWayWithTransform) {
    Property<float> source("source", 5.0f);
    Property<std::string> target("target", "");

    Binding<float, std::string> binding(source, target,
        [](const float& v) -> std::string {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0f%%", v);
            return buf;
        });

    EXPECT_EQ(target.Get(), "5%");
    source.Set(75.0f);
    EXPECT_EQ(target.Get(), "75%");
}

TEST(BindingTest, Unbind) {
    Property<float> source("source", 1.0f);
    Property<float> target("target", 0.0f);

    Binding<float, float> binding(source, target);
    binding.Unbind();
    source.Set(99.0f);
    EXPECT_FLOAT_EQ(target.Get(), 1.0f); // no propagation after unbind
}

TEST(BindingTest, BidirectionalSync) {
    Property<float> a("a", 0.0f);
    Property<float> b("b", 0.0f);

    BidirectionalBinding<float> binding(a, b);
    a.Set(10.0f);
    EXPECT_FLOAT_EQ(b.Get(), 10.0f);

    b.Set(20.0f);
    EXPECT_FLOAT_EQ(a.Get(), 20.0f);
}

TEST(BindingTest, BidirectionalNoCycle) {
    Property<float> a("a", 0.0f);
    Property<float> b("b", 0.0f);

    BidirectionalBinding<float> binding(a, b);
    // Should not infinite loop
    a.Set(5.0f);
    EXPECT_FLOAT_EQ(a.Get(), 5.0f);
    EXPECT_FLOAT_EQ(b.Get(), 5.0f);
}

TEST(BindingTest, BindingScopeRAII) {
    Property<float> source("source", 0.0f);
    Property<float> target("target", 0.0f);

    {
        BindingScope scope;
        scope.Bind(source, target);
        source.Set(10.0f);
        EXPECT_FLOAT_EQ(target.Get(), 10.0f);
    }
    // scope destroyed, binding unbound
    source.Set(20.0f);
    EXPECT_FLOAT_EQ(target.Get(), 10.0f);
}

TEST(BindingTest, BindFormat) {
    Property<float> source("source", 42.0f);
    Property<std::string> target("target", "");

    BindingScope scope;
    scope.BindFormat(source, target, "Value: %.0f");
    EXPECT_EQ(target.Get(), "Value: 42");

    source.Set(100.0f);
    EXPECT_EQ(target.Get(), "Value: 100");
}

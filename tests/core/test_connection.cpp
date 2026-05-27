#include <gtest/gtest.h>
#include <core/connection.h>

using namespace CarHMI::Core;

TEST(ConnectionTest, DefaultIsDisconnected) {
    Connection c;
    EXPECT_FALSE(c.IsConnected());
}

TEST(ConnectionTest, ConstructWithCallback) {
    bool called = false;
    {
        Connection c([&called]() { called = true; });
        EXPECT_TRUE(c.IsConnected());
    }
    EXPECT_TRUE(called);
}

TEST(ConnectionTest, DisconnectCallsOnce) {
    int count = 0;
    Connection c([&count]() { count++; });
    c.Disconnect();
    EXPECT_EQ(count, 1);
    c.Disconnect(); // second call should be no-op
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(c.IsConnected());
}

TEST(ConnectionTest, MoveConstructor) {
    bool called = false;
    Connection c1([&called]() { called = true; });
    Connection c2(std::move(c1));
    EXPECT_FALSE(c1.IsConnected());
    EXPECT_TRUE(c2.IsConnected());
    c2.Disconnect();
    EXPECT_TRUE(called);
}

TEST(ConnectionTest, MoveAssignment) {
    bool called1 = false, called2 = false;
    Connection c1([&called1]() { called1 = true; });
    Connection c2([&called2]() { called2 = true; });
    c1 = std::move(c2);
    EXPECT_TRUE(called1); // old c1 disconnected
    EXPECT_FALSE(called2);
    c1.Disconnect();
    EXPECT_TRUE(called2);
}

TEST(ConnectionTest, Release) {
    bool called = false;
    Connection c1([&called]() { called = true; });
    Connection c2 = c1.Release();
    EXPECT_FALSE(c1.IsConnected());
    EXPECT_TRUE(c2.IsConnected());
    EXPECT_FALSE(called);
    c2.Disconnect();
    EXPECT_TRUE(called);
}

TEST(ConnectionGroupTest, AddAndDisconnectAll) {
    int count = 0;
    ConnectionGroup group;
    group.Add(Connection([&count]() { count++; }));
    group.Add(Connection([&count]() { count++; }));
    EXPECT_EQ(group.Size(), 2u);
    group.DisconnectAll();
    EXPECT_EQ(count, 2);
    EXPECT_EQ(group.Size(), 0u);
}

TEST(ConnectionGroupTest, SkipsDisconnected) {
    ConnectionGroup group;
    Connection c; // default, not connected
    group.Add(std::move(c));
    EXPECT_EQ(group.Size(), 0u);
}

TEST(ConnectionGroupTest, DestructorDisconnects) {
    int count = 0;
    {
        ConnectionGroup group;
        group.Add(Connection([&count]() { count++; }));
    }
    EXPECT_EQ(count, 1);
}

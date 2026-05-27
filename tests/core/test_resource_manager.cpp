#include <gtest/gtest.h>
#include <core/resource_manager.h>

using namespace CarHMI::Core;

struct TestResource {
    std::string data;
    TestResource(const std::string& d) : data(d) {}
};

class ResourceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto& rm = ResourceManager::Get();
        rm.Clear();
        rm.RegisterLoader<TestResource>([](const std::string& path) {
            return std::make_shared<TestResource>("loaded:" + path);
        });
    }
    void TearDown() override {
        ResourceManager::Get().Clear();
    }
};

TEST_F(ResourceManagerTest, LoadReturnsResource) {
    auto res = ResourceManager::Get().Load<TestResource>("test.dat");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->data, "loaded:test.dat");
}

TEST_F(ResourceManagerTest, CacheHit) {
    auto res1 = ResourceManager::Get().Load<TestResource>("test.dat");
    auto res2 = ResourceManager::Get().Load<TestResource>("test.dat");
    EXPECT_EQ(res1.get(), res2.get()); // same pointer
}

TEST_F(ResourceManagerTest, CacheExpires) {
    auto res1 = ResourceManager::Get().Load<TestResource>("test.dat");
    res1.reset(); // drop reference
    ResourceManager::Get().CollectGarbage();

    auto res2 = ResourceManager::Get().Load<TestResource>("test.dat");
    // Should be a fresh load (new pointer)
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->data, "loaded:test.dat");
}

TEST_F(ResourceManagerTest, CollectGarbage) {
    {
        auto res = ResourceManager::Get().Load<TestResource>("temp.dat");
    }
    EXPECT_EQ(ResourceManager::Get().CachedCount(), 1u);
    EXPECT_EQ(ResourceManager::Get().AliveCount(), 0u);
    ResourceManager::Get().CollectGarbage();
    EXPECT_EQ(ResourceManager::Get().CachedCount(), 0u);
}

TEST_F(ResourceManagerTest, Clear) {
    auto res = ResourceManager::Get().Load<TestResource>("test.dat");
    ResourceManager::Get().Clear();
    EXPECT_EQ(ResourceManager::Get().CachedCount(), 0u);
}

TEST_F(ResourceManagerTest, AliveCount) {
    auto res = ResourceManager::Get().Load<TestResource>("a.dat");
    {
        auto res2 = ResourceManager::Get().Load<TestResource>("b.dat");
        EXPECT_EQ(ResourceManager::Get().AliveCount(), 2u);
    }
    EXPECT_EQ(ResourceManager::Get().AliveCount(), 1u);
}

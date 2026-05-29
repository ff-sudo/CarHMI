#include <gtest/gtest.h>
#include <runtime/data_model.h>
#include <runtime/sim_data_source.h>
#include <runtime/state_machine.h>
#include <nlohmann/json.hpp>

using namespace CarHMI::Runtime;
using json = nlohmann::json;

// --- DataModel Tests ---

TEST(DataModelTest, RegisterAndGetFloat) {
    DataModel model("test");
    auto* prop = model.RegisterFloat("speed", 60.0f);
    ASSERT_NE(prop, nullptr);
    EXPECT_FLOAT_EQ(prop->Get(), 60.0f);
    EXPECT_FLOAT_EQ(model.GetFloat("speed")->Get(), 60.0f);
}

TEST(DataModelTest, RegisterAndGetString) {
    DataModel model("test");
    model.RegisterString("name", "hello");
    auto* prop = model.GetString("name");
    ASSERT_NE(prop, nullptr);
    EXPECT_EQ(prop->Get(), "hello");
}

TEST(DataModelTest, LoadFromJson) {
    DataModel model("test");
    json j = {{"speed", 80.5}, {"fuel", 0.75}, {"label", "OK"}, {"active", true}};
    model.LoadFromJson(j);

    EXPECT_FLOAT_EQ(model.GetFloat("speed")->Get(), 80.5f);
    EXPECT_FLOAT_EQ(model.GetFloat("fuel")->Get(), 0.75f);
    EXPECT_EQ(model.GetString("label")->Get(), "OK");
    EXPECT_FLOAT_EQ(model.GetFloat("active")->Get(), 1.0f); // bool → float
}

TEST(DataModelTest, SetFloat) {
    DataModel model("test");
    model.RegisterFloat("x", 0);
    EXPECT_TRUE(model.SetFloat("x", 42.0f));
    EXPECT_FLOAT_EQ(model.GetFloat("x")->Get(), 42.0f);
    EXPECT_FALSE(model.SetFloat("nonexist", 1.0f));
}

TEST(DataModelTest, Registry) {
    DataModelRegistry::Get().Clear();
    auto* m1 = DataModelRegistry::Get().Create("car");
    m1->RegisterFloat("speed", 100);

    auto* found = DataModelRegistry::Get().Find("car");
    EXPECT_EQ(found, m1);
    EXPECT_EQ(DataModelRegistry::Get().Find("nope"), nullptr);

    auto* m2 = DataModelRegistry::Get().GetOrCreate("car");
    EXPECT_EQ(m2, m1); // same instance

    DataModelRegistry::Get().Clear();
}

// --- SimDataSource Tests ---

TEST(SimDataSourceTest, LoadAndTick) {
    DataModelRegistry::Get().Clear();
    auto* model = DataModelRegistry::Get().Create("sim");
    model->RegisterFloat("val", 0);

    json config = {
        {"model", "sim"},
        {"interval", 100},
        {"properties", {{
            {"name", "val"},
            {"mode", "fixed"},
            {"value", 42.0}
        }}}
    };

    SimDataSource src;
    src.LoadFromJson(config);
    src.Tick(); // manual tick
    EXPECT_FLOAT_EQ(model->GetFloat("val")->Get(), 42.0f);
    DataModelRegistry::Get().Clear();
}

TEST(SimDataSourceTest, SineMode) {
    DataModelRegistry::Get().Clear();

    json config = {
        {"model", "sine_test"},
        {"interval", 1000},
        {"properties", {{
            {"name", "wave"},
            {"mode", "sine"},
            {"min", 0},
            {"max", 100},
            {"period", 2.0}
        }}}
    };

    SimDataSource src;
    src.LoadFromJson(config);
    src.Start(); // creates model + property
    src.Tick();

    auto* model = DataModelRegistry::Get().Find("sine_test");
    ASSERT_NE(model, nullptr);
    float val = model->GetFloat("wave")->Get();
    EXPECT_GE(val, 0.0f);
    EXPECT_LE(val, 100.0f);

    src.Stop();
    DataModelRegistry::Get().Clear();
}

// --- StateMachine Tests ---

TEST(StateMachineTest, LoadAndTransition) {
    DataModelRegistry::Get().Clear();
    auto* model = DataModelRegistry::Get().Create("sm");
    model->RegisterFloat("speed", 0);

    json config = {
        {"model", "sm"},
        {"initial", "idle"},
        {"states", {
            {"idle", {{"transitions", {{{"to", "driving"}, {"when", {{"property", "speed"}, {"gt", 0}}}}}}}},
            {"driving", {{"transitions", {{{"to", "idle"}, {"when", {{"property", "speed"}, {"eq", 0}}}}}}}}
        }}
    };

    StateMachine sm;
    sm.LoadFromJson(config);
    EXPECT_EQ(sm.GetCurrentState(), "idle");

    sm.Update(); // speed=0, no transition
    EXPECT_EQ(sm.GetCurrentState(), "idle");

    model->SetFloat("speed", 50);
    sm.Update(); // speed>0, should transition to driving
    EXPECT_EQ(sm.GetCurrentState(), "driving");

    model->SetFloat("speed", 0);
    sm.Update(); // speed==0, back to idle
    EXPECT_EQ(sm.GetCurrentState(), "idle");

    DataModelRegistry::Get().Clear();
}

TEST(StateMachineTest, StateChangeCallback) {
    DataModelRegistry::Get().Clear();
    auto* model = DataModelRegistry::Get().Create("cb");
    model->RegisterFloat("x", 0);

    json config = {
        {"model", "cb"},
        {"initial", "a"},
        {"states", {
            {"a", {{"transitions", {{{"to", "b"}, {"when", {{"property", "x"}, {"gte", 10}}}}}}}},
            {"b", {{"transitions", json::array()}}}
        }}
    };

    StateMachine sm;
    sm.LoadFromJson(config);

    std::string fromResult, toResult;
    sm.OnStateChange([&](const std::string& from, const std::string& to) {
        fromResult = from;
        toResult = to;
    });

    model->SetFloat("x", 15);
    sm.Update();
    EXPECT_EQ(fromResult, "a");
    EXPECT_EQ(toResult, "b");

    DataModelRegistry::Get().Clear();
}

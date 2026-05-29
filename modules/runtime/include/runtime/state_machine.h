#pragma once

#include "data_model.h"
#include <core/connection.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace CarHMI::Runtime {

/// StateMachine: manages states and conditional transitions, driven by DataModel properties.
///
/// JSON format:
/// {
///   "initial": "idle",
///   "model": "vehicle",
///   "states": {
///     "idle": {
///       "transitions": [
///         { "to": "driving", "when": { "property": "speed", "gt": 0 } }
///       ]
///     },
///     "driving": {
///       "transitions": [
///         { "to": "idle", "when": { "property": "speed", "eq": 0 } },
///         { "to": "speeding", "when": { "property": "speed", "gt": 120 } }
///       ]
///     },
///     "speeding": {
///       "transitions": [
///         { "to": "driving", "when": { "property": "speed", "lte": 120 } }
///       ]
///     }
///   }
/// }
///
/// Condition operators: "eq", "neq", "gt", "gte", "lt", "lte"
class StateMachine {
public:
    using StateChangeCallback = std::function<void(const std::string& from, const std::string& to)>;

    struct Condition {
        std::string property;
        enum Op { Eq, Neq, Gt, Gte, Lt, Lte } op = Eq;
        float value = 0;
    };

    struct Transition {
        std::string to;
        Condition condition;
    };

    struct State {
        std::string name;
        std::vector<Transition> transitions;
    };

    void LoadFromJson(const nlohmann::json& j) {
        m_modelName = j.value("model", "default");
        m_currentState = j.value("initial", "");

        if (j.contains("states") && j["states"].is_object()) {
            for (auto& [name, sj] : j["states"].items()) {
                State state;
                state.name = name;

                if (sj.contains("transitions") && sj["transitions"].is_array()) {
                    for (auto& tj : sj["transitions"]) {
                        Transition t;
                        t.to = tj.value("to", "");

                        if (tj.contains("when") && tj["when"].is_object()) {
                            auto& wj = tj["when"];
                            t.condition.property = wj.value("property", "");
                            t.condition.value = wj.value("value", 0.0f);

                            // Parse operator from keys
                            if (wj.contains("eq")) { t.condition.op = Condition::Eq; t.condition.value = wj["eq"].get<float>(); }
                            else if (wj.contains("neq")) { t.condition.op = Condition::Neq; t.condition.value = wj["neq"].get<float>(); }
                            else if (wj.contains("gt")) { t.condition.op = Condition::Gt; t.condition.value = wj["gt"].get<float>(); }
                            else if (wj.contains("gte")) { t.condition.op = Condition::Gte; t.condition.value = wj["gte"].get<float>(); }
                            else if (wj.contains("lt")) { t.condition.op = Condition::Lt; t.condition.value = wj["lt"].get<float>(); }
                            else if (wj.contains("lte")) { t.condition.op = Condition::Lte; t.condition.value = wj["lte"].get<float>(); }
                        }

                        state.transitions.push_back(std::move(t));
                    }
                }

                m_states[name] = std::move(state);
            }
        }
    }

    /// Evaluate transitions based on current DataModel state. Call this each frame or on data change.
    void Update() {
        if (m_currentState.empty()) return;

        auto it = m_states.find(m_currentState);
        if (it == m_states.end()) return;

        auto* model = DataModelRegistry::Get().Find(m_modelName);
        if (!model) return;

        for (auto& t : it->second.transitions) {
            if (EvaluateCondition(t.condition, *model)) {
                std::string from = m_currentState;
                m_currentState = t.to;
                spdlog::info("StateMachine: {} → {}", from, m_currentState);
                if (m_onStateChange) m_onStateChange(from, m_currentState);
                break; // Only one transition per frame
            }
        }
    }

    const std::string& GetCurrentState() const { return m_currentState; }
    void SetState(const std::string& state) { m_currentState = state; }
    void OnStateChange(StateChangeCallback cb) { m_onStateChange = std::move(cb); }

    bool HasState(const std::string& name) const { return m_states.count(name) > 0; }
    const std::string& GetModelName() const { return m_modelName; }

private:
    bool EvaluateCondition(const Condition& c, DataModel& model) {
        if (c.property.empty()) return false;
        auto* prop = model.GetFloat(c.property);
        if (!prop) return false;

        float val = prop->Get();
        switch (c.op) {
            case Condition::Eq:  return val == c.value;
            case Condition::Neq: return val != c.value;
            case Condition::Gt:  return val > c.value;
            case Condition::Gte: return val >= c.value;
            case Condition::Lt:  return val < c.value;
            case Condition::Lte: return val <= c.value;
        }
        return false;
    }

    std::string m_modelName;
    std::string m_currentState;
    std::unordered_map<std::string, State> m_states;
    StateChangeCallback m_onStateChange;
};

} // namespace CarHMI::Runtime

#pragma once

#include "data_model.h"
#include <core/timer/timer_manager.h>
#include <core/connection.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <random>
#include <cmath>

namespace CarHMI::Runtime {

/// SimDataSource: periodically updates DataModel properties with simulated values.
/// Configured via JSON: each entry defines a property, update mode, and parameters.
///
/// JSON format:
/// {
///   "model": "vehicle",
///   "interval": 500,
///   "properties": [
///     { "name": "speed", "mode": "sine", "min": 0, "max": 120, "period": 5.0 },
///     { "name": "rpm", "mode": "random", "min": 800, "max": 6000 },
///     { "name": "fuel", "mode": "linear", "start": 100, "end": 0, "duration": 60 },
///     { "name": "gear", "mode": "sequence", "values": [1, 2, 3, 4, 5] }
///   ]
/// }
class SimDataSource {
public:
    enum class Mode { Sine, Random, Linear, Sequence, Fixed };

    struct PropertySim {
        std::string name;
        Mode mode = Mode::Fixed;
        float min = 0, max = 1;
        float period = 1.0f;      // seconds (for sine)
        float start = 0, end = 1; // for linear
        float duration = 10.0f;   // seconds (for linear)
        std::vector<float> values; // for sequence
        float fixedValue = 0;
    };

    SimDataSource() : m_rng(std::random_device{}()) {}

    void LoadFromJson(const nlohmann::json& j) {
        m_modelName = j.value("model", "default");
        m_intervalMs = j.value("interval", 500);

        if (j.contains("properties") && j["properties"].is_array()) {
            for (auto& pj : j["properties"]) {
                PropertySim ps;
                ps.name = pj.value("name", "");
                if (ps.name.empty()) continue;

                std::string mode = pj.value("mode", "fixed");
                if (mode == "sine") ps.mode = Mode::Sine;
                else if (mode == "random") ps.mode = Mode::Random;
                else if (mode == "linear") ps.mode = Mode::Linear;
                else if (mode == "sequence") ps.mode = Mode::Sequence;
                else ps.mode = Mode::Fixed;

                ps.min = pj.value("min", 0.0f);
                ps.max = pj.value("max", 1.0f);
                ps.period = pj.value("period", 1.0f);
                ps.start = pj.value("start", 0.0f);
                ps.end = pj.value("end", 1.0f);
                ps.duration = pj.value("duration", 10.0f);
                ps.fixedValue = pj.value("value", 0.0f);

                if (pj.contains("values") && pj["values"].is_array()) {
                    for (auto& v : pj["values"])
                        ps.values.push_back(v.get<float>());
                }

                m_sims.push_back(std::move(ps));
            }
        }
    }

    /// Start the simulation. Registers properties in DataModel if they don't exist.
    void Start() {
        auto* model = DataModelRegistry::Get().GetOrCreate(m_modelName);

        // Ensure all properties exist
        for (auto& ps : m_sims) {
            if (!model->GetFloat(ps.name))
                model->RegisterFloat(ps.name, GetInitialValue(ps));
        }

        m_elapsed = 0;
        m_sequenceIndex = 0;

        m_timer = Core::TimerManager::Get().SetInterval([this]() {
            Tick();
        }, m_intervalMs);
    }

    void Stop() {
        m_timer.Disconnect();
    }

    /// Manual tick (useful for testing without timer)
    void Tick() {
        auto* model = DataModelRegistry::Get().Find(m_modelName);
        if (!model) return;

        m_elapsed += m_intervalMs / 1000.0f;

        for (size_t i = 0; i < m_sims.size(); i++) {
            auto& ps = m_sims[i];
            float value = ComputeValue(ps, i);
            model->SetFloat(ps.name, value);
        }

        m_sequenceIndex++;
    }

    const std::string& GetModelName() const { return m_modelName; }
    int GetIntervalMs() const { return m_intervalMs; }
    const std::vector<PropertySim>& GetSims() const { return m_sims; }

private:
    float GetInitialValue(const PropertySim& ps) {
        switch (ps.mode) {
            case Mode::Sine: return (ps.min + ps.max) / 2.0f;
            case Mode::Random: return ps.min;
            case Mode::Linear: return ps.start;
            case Mode::Sequence: return ps.values.empty() ? 0 : ps.values[0];
            case Mode::Fixed: return ps.fixedValue;
        }
        return 0;
    }

    float ComputeValue(const PropertySim& ps, size_t index) {
        switch (ps.mode) {
            case Mode::Sine: {
                float t = std::sin(m_elapsed * 2.0f * 3.14159f / ps.period);
                return ps.min + (ps.max - ps.min) * (t * 0.5f + 0.5f);
            }
            case Mode::Random: {
                std::uniform_real_distribution<float> dist(ps.min, ps.max);
                return dist(m_rng);
            }
            case Mode::Linear: {
                float t = std::min(m_elapsed / ps.duration, 1.0f);
                return ps.start + (ps.end - ps.start) * t;
            }
            case Mode::Sequence: {
                if (ps.values.empty()) return 0;
                return ps.values[m_sequenceIndex % ps.values.size()];
            }
            case Mode::Fixed:
                return ps.fixedValue;
        }
        return 0;
    }

    std::string m_modelName = "default";
    int m_intervalMs = 500;
    std::vector<PropertySim> m_sims;
    float m_elapsed = 0;
    size_t m_sequenceIndex = 0;
    std::mt19937 m_rng;
    Core::Connection m_timer;
};

} // namespace CarHMI::Runtime

/**
* @file AgentInternal.h
 * @author ezazl
 *
 * @brief
 * Agent internal dynamics engine.
 *
 * Owns an agent's Tendencies (slow traits) and State (fast pressures), and updates
 * the State each tick as time passes. Optionally allows slow drift (plasticity)
 * where Tendencies adapt based on chronic State.
 */

#ifndef AGENTINTERNAL_H
#define AGENTINTERNAL_H

#include <string>
#include "AgentData.h"

namespace Group5 {

class AgentInternal {
public:
    /// Create an internal engine with a debug name and initial tendencies.
    AgentInternal(std::string name, Tendencies tendencies);

    /// Advance internal time by dt and update State accordingly.
    void Tick(float dt);

    /// Enable/disable slow tendency drift (plasticity).
    void SetPlasticity(bool on);

    /// Read-only access to tendencies (traits).
    const Tendencies& GetTendencies() const;

    /// Read-only access to current internal state (pressures).
    const State& GetState() const;

    /// Current internal time (seconds or arbitrary time units).
    float GetTime() const;

    /// Debug print of state + tendencies.
    void PrintLine() const;

private:
    // --- Owned data ---
    std::string name_;
    Tendencies T_{};
    State S_{};
    float time_ = 0.0f;
    bool plasticity_ = false;

    // --- Helper utilities ---
    static float Clamp01(float x);
    static float Lerp(float a, float b, float t);

    // Slow adaptation (only used when plasticity_ is true).
    void UpdateTendenciesSlow(float dt);
};

} // namespace Group5

#endif // AGENTINTERNAL_H

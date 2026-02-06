/**
 * @file AgentInternal.cpp
 * @author ezazl
 */

#include "AgentInternal.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace Group5 {

AgentInternal::AgentInternal(std::string name, Tendencies tendencies)
    : name_(std::move(name)), T_(tendencies) {}

void AgentInternal::SetPlasticity(bool on) { plasticity_ = on; }

const Tendencies& AgentInternal::GetTendencies() const { return T_; }
const State& AgentInternal::GetState() const { return S_; }
float AgentInternal::GetTime() const { return time_; }

float AgentInternal::Clamp01(float x) { return std::clamp(x, 0.0f, 1.0f); }
float AgentInternal::Lerp(float a, float b, float t) { return a + (b - a) * t; }

void AgentInternal::Tick(float dt) {
  dt = std::max(dt, 0.0f);

  // Helper: gently pull a state variable toward a baseline setpoint.
  auto pull_to = [&](float& x, float target, float rate) {
    x = Clamp01(x + (target - x) * rate * dt);
  };

  // --- Homeostatic setpoints (where the agent tends to settle, absent events) ---
  constexpr float bore_set = 0.20f;
  constexpr float lone_set = 0.15f;
  constexpr float anx_set  = 0.10f;
  constexpr float fat_set  = 0.20f;

  // --- Regulation toward baseline ---
  pull_to(S_.boredom,     bore_set, 0.60f);
  pull_to(S_.loneliness,  lone_set, 0.50f);
  pull_to(S_.anxiety,     anx_set,  0.35f);
  pull_to(S_.fatigue,     fat_set,  0.55f);

  // --- Unmet-need pressures (tendencies cause needs to build over time) ---
  // Higher curiosity => boredom builds faster if "under-stimulated".
  S_.boredom = Clamp01(S_.boredom + dt * 0.18f * T_.curiosity);

  // Higher love => loneliness builds faster if "under-connected".
  S_.loneliness = Clamp01(S_.loneliness + dt * 0.16f * T_.love);

  // Fear increases baseline anxiety under uncertainty; satisfaction buffers it.
  {
    float uncertainty = 0.10f;                 // placeholder until world signals exist
    float buffer = 0.25f * T_.satisfaction;    // trait-level resilience
    S_.anxiety = Clamp01(
        S_.anxiety + dt * uncertainty
        * (0.25f + 0.85f * T_.fear)
        * (1.0f - buffer));
  }

  // Fatigue accumulates; comfort makes energy cost more salient.
  S_.fatigue = Clamp01(S_.fatigue + dt * 0.12f * (0.4f + 0.6f * T_.comfort));

  // Fulfillment decays slowly; satisfaction slows decay a bit.
  S_.fulfillment = Clamp01(S_.fulfillment - dt * 0.10f * (1.0f - 0.5f * T_.satisfaction));

  // --- Couplings (internal interactions) ---
  // tired -> anxious; fulfillment buffers loneliness/anxiety.
  S_.anxiety    = Clamp01(S_.anxiety + dt * 0.10f * S_.fatigue);
  S_.loneliness = Clamp01(S_.loneliness - dt * 0.08f * S_.fulfillment);
  S_.anxiety    = Clamp01(S_.anxiety    - dt * 0.05f * S_.fulfillment);

  // Optional slow trait drift.
  if (plasticity_) UpdateTendenciesSlow(dt);

  time_ += dt;
}

void AgentInternal::UpdateTendenciesSlow(float dt) {
  const float lr = 0.0025f; // tiny drift; keep traits stable over short runs

  // chronic anxiety -> fear rises (learned caution)
  float fear_target = Clamp01(0.25f + 0.75f * S_.anxiety);
  T_.fear = Clamp01(Lerp(T_.fear, fear_target, lr * dt));

  // chronic loneliness -> love rises (stronger affiliation drive)
  float love_target = Clamp01(0.20f + 0.80f * S_.loneliness);
  T_.love = Clamp01(Lerp(T_.love, love_target, lr * dt));

  // chronic fatigue -> comfort rises (learned preference for rest/stability)
  float comfort_target = Clamp01(0.30f + 0.70f * S_.fatigue);
  T_.comfort = Clamp01(Lerp(T_.comfort, comfort_target, lr * dt));

  // stable fulfillment -> satisfaction rises (learned resilience)
  float sat_target = Clamp01(0.30f + 0.70f * (1.0f - S_.anxiety) * S_.fulfillment);
  T_.satisfaction = Clamp01(Lerp(T_.satisfaction, sat_target, lr * dt));

  // boredom -> curiosity rises slightly (restlessness increases exploration bias)
  float cur_target = Clamp01(0.25f + 0.75f * S_.boredom);
  T_.curiosity = Clamp01(Lerp(T_.curiosity, cur_target, lr * dt));
}

void AgentInternal::PrintLine() const {
  std::cout << std::fixed << std::setprecision(2)
            << std::setw(8) << time_ << " | " << std::setw(10) << name_
            << " | S[bore " << S_.boredom
            << " lone " << S_.loneliness
            << " anx "  << S_.anxiety
            << " fat "  << S_.fatigue
            << " fulf " << S_.fulfillment << "]"
            << " | T[cur " << T_.curiosity
            << " love " << T_.love
            << " fear " << T_.fear
            << " comf " << T_.comfort
            << " sat "  << T_.satisfaction << "]\n";
}

} // namespace Group5

/**
 * @file phase_engine.cpp
 * @brief Implementation of Phase Detection Engine
 */

#include "phase_engine.h"
#include <Arduino.h>
#include <string.h>

namespace UCF {

// Phase color definitions (RGB)
static const uint8_t PHASE_COLORS[3][3] = {
    {255, 80, 50},    // UNTRUE: Red/Orange
    {220, 255, 80},   // PARADOX: Yellow/Green
    {80, 200, 255}    // TRUE: Cyan
};

// Tier to Solfeggio frequency lookup
static const uint16_t TIER_FREQUENCIES[9] = {
    Solfeggio::UT,      // t1: 174 Hz
    Solfeggio::RE,      // t2: 285 Hz
    Solfeggio::MI,      // t3: 396 Hz
    Solfeggio::FA,      // t4: 417 Hz
    Solfeggio::SOL,     // t5: 528 Hz
    Solfeggio::LA,      // t6: 639 Hz
    Solfeggio::SI,      // t7: 741 Hz
    Solfeggio::DO,      // t8: 852 Hz
    Solfeggio::RE_HIGH  // t9: 963 Hz
};

PhaseEngine::PhaseEngine()
    : m_transition_flag(false)
    , m_alpha(0.1f)
    , m_stability_threshold(500)
    , m_z_prev(0.0f)
    , m_time_prev(0)
    , m_history_head(0)
    , m_history_count(0)
{
    // Initialize state
    memset(&m_state, 0, sizeof(m_state));
    m_state.current = Phase::UNTRUE;
    m_state.previous = Phase::UNTRUE;
    m_state.tier = 1;

    memset(&m_last_transition, 0, sizeof(m_last_transition));
    memset(m_history, 0, sizeof(m_history));
}

bool PhaseEngine::begin() {
    // Configure indicator LED pins
    pinMode(Pins::LED_UNTRUE, OUTPUT);
    pinMode(Pins::LED_PARADOX, OUTPUT);
    pinMode(Pins::LED_TRUE, OUTPUT);

    // Initial state: UNTRUE
    digitalWrite(Pins::LED_UNTRUE, HIGH);
    digitalWrite(Pins::LED_PARADOX, LOW);
    digitalWrite(Pins::LED_TRUE, LOW);

    m_time_prev = millis();

    return true;
}

void PhaseEngine::update(const HexFieldState& field) {
    updateFromZ(field.z);
}

void PhaseEngine::updateFromZ(float z) {
    uint32_t now = millis();
    float dt = (now - m_time_prev) / 1000.0f;

    // Store raw z
    m_state.z = z;

    // Apply EMA smoothing
    m_state.z_smoothed = m_alpha * z + (1.0f - m_alpha) * m_state.z_smoothed;

    // Compute velocity
    if (dt > 0.001f) {
        m_state.z_velocity = (z - m_z_prev) / dt;
    }

    // Detect phase from smoothed z
    Phase detected = detectPhase(m_state.z_smoothed);

    // Check for transition
    m_transition_flag = false;
    if (detected != m_state.current) {
        recordTransition(m_state.current, detected, m_state.z_smoothed);
        m_state.previous = m_state.current;
        m_state.current = detected;
        m_state.last_transition = now;
        m_state.phase_duration = 0;
        m_transition_flag = true;
    } else {
        m_state.phase_duration = now - m_state.last_transition;
    }

    // Update stability flag
    m_state.is_stable = (m_state.phase_duration >= m_stability_threshold);

    // Update tier
    m_state.tier = z_to_tier(m_state.z_smoothed);

    // Add to history
    addToHistory(z, m_state.current);

    // Update for next iteration
    m_z_prev = z;
    m_time_prev = now;
}

Phase PhaseEngine::detectPhase(float z) {
    // Apply hysteresis to prevent oscillation at boundaries
    const float HYSTERESIS = 0.02f;

    Phase current = m_state.current;

    switch (current) {
        case Phase::UNTRUE:
            // Rising: need to exceed upper boundary
            if (z >= PHI_INV + HYSTERESIS) {
                return Phase::PARADOX;
            }
            break;

        case Phase::PARADOX:
            // Falling: need to drop below lower boundary
            if (z < PHI_INV - HYSTERESIS) {
                return Phase::UNTRUE;
            }
            // Rising: need to exceed z_critical
            if (z >= Z_CRITICAL + HYSTERESIS) {
                return Phase::TRUE;
            }
            break;

        case Phase::TRUE:
            // Falling: need to drop below z_critical
            if (z < Z_CRITICAL - HYSTERESIS) {
                return Phase::PARADOX;
            }
            break;
    }

    return current;
}

void PhaseEngine::recordTransition(Phase from, Phase to, float z) {
    m_last_transition.from = from;
    m_last_transition.to = to;
    m_last_transition.z_at_transition = z;
    m_last_transition.timestamp = millis();
}

void PhaseEngine::addToHistory(float z, Phase phase) {
    m_history[m_history_head].z = z;
    m_history[m_history_head].phase = phase;
    m_history[m_history_head].timestamp = millis();

    m_history_head = (m_history_head + 1) % HISTORY_SIZE;
    if (m_history_count < HISTORY_SIZE) {
        m_history_count++;
    }
}

uint16_t PhaseEngine::getCurrentFrequency() const {
    if (m_state.tier >= 1 && m_state.tier <= 9) {
        return TIER_FREQUENCIES[m_state.tier - 1];
    }
    return Solfeggio::UT;  // Default
}

void PhaseEngine::getCurrentColor(uint8_t& r, uint8_t& g, uint8_t& b) const {
    uint8_t phase_idx = static_cast<uint8_t>(m_state.current);
    if (phase_idx > 2) phase_idx = 0;

    r = PHASE_COLORS[phase_idx][0];
    g = PHASE_COLORS[phase_idx][1];
    b = PHASE_COLORS[phase_idx][2];

    // Modulate by tier within phase for gradient effect
    float tier_mod = 1.0f;
    uint8_t base_tier = (phase_idx == 0) ? 1 : (phase_idx == 1) ? 4 : 7;
    uint8_t tier_in_phase = m_state.tier - base_tier;

    if (tier_in_phase > 0 && tier_in_phase < 3) {
        tier_mod = 0.8f + 0.1f * tier_in_phase;
    }

    r = static_cast<uint8_t>(r * tier_mod);
    g = static_cast<uint8_t>(g * tier_mod);
    b = static_cast<uint8_t>(b * tier_mod);
}

void PhaseEngine::setSmoothingFactor(float alpha) {
    if (alpha < 0.01f) alpha = 0.01f;
    if (alpha > 1.0f) alpha = 1.0f;
    m_alpha = alpha;
}

void PhaseEngine::setStabilityThreshold(uint32_t ms) {
    m_stability_threshold = ms;
}

void PhaseEngine::updateIndicators() {
    digitalWrite(Pins::LED_UNTRUE, m_state.current == Phase::UNTRUE ? HIGH : LOW);
    digitalWrite(Pins::LED_PARADOX, m_state.current == Phase::PARADOX ? HIGH : LOW);
    digitalWrite(Pins::LED_TRUE, m_state.current == Phase::TRUE ? HIGH : LOW);
}

uint16_t PhaseEngine::getHistory(PhaseHistoryEntry* buffer, uint16_t maxEntries) {
    uint16_t count = (maxEntries < m_history_count) ? maxEntries : m_history_count;

    // Copy from oldest to newest
    uint16_t start = (m_history_head + HISTORY_SIZE - m_history_count) % HISTORY_SIZE;

    for (uint16_t i = 0; i < count; i++) {
        uint16_t idx = (start + i) % HISTORY_SIZE;
        buffer[i] = m_history[idx];
    }

    return count;
}

const char* phaseToString(Phase phase) {
    switch (phase) {
        case Phase::UNTRUE:  return "UNTRUE";
        case Phase::PARADOX: return "PARADOX";
        case Phase::TRUE:    return "TRUE";
        default:             return "UNKNOWN";
    }
}

float getPhaseVoltage(uint8_t boundary, float vref) {
    if (boundary == 0) {
        // UNTRUE/PARADOX at phi^-1
        return vref * PHI_INV;
    } else {
        // PARADOX/TRUE at z_critical
        return vref * Z_CRITICAL;
    }
}

} // namespace UCF

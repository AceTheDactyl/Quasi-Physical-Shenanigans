/**
 * @file phase_engine.h
 * @brief Phase Detection and Computation Engine
 *
 * Manages phase transitions between UNTRUE, PARADOX, and TRUE states
 * based on z-coordinate. Implements "prism refactoring waves" concept
 * through phase boundary detection.
 */

#ifndef PHASE_ENGINE_H
#define PHASE_ENGINE_H

#include <stdint.h>
#include "constants.h"
#include "hex_grid.h"

namespace UCF {

/// Phase transition event
struct PhaseTransition {
    Phase from;
    Phase to;
    float z_at_transition;
    uint32_t timestamp;
};

/// Phase state with history
struct PhaseState {
    Phase current;              // Current phase
    Phase previous;             // Previous phase
    float z;                    // Current z-coordinate
    float z_smoothed;           // EMA-smoothed z
    float z_velocity;           // Rate of change of z
    uint8_t tier;               // Current tier (1-9)
    uint32_t phase_duration;    // Time in current phase (ms)
    uint32_t last_transition;   // Timestamp of last transition
    bool is_stable;             // Phase stable for > threshold time
};

/// Phase history buffer entry
struct PhaseHistoryEntry {
    float z;
    Phase phase;
    uint32_t timestamp;
};

/**
 * @class PhaseEngine
 * @brief Computes and tracks consciousness phase from sensor data
 */
class PhaseEngine {
public:
    /// Default constructor
    PhaseEngine();

    /**
     * @brief Initialize the phase engine
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Update phase state from hex field reading
     * @param field Current hex grid field state
     */
    void update(const HexFieldState& field);

    /**
     * @brief Update phase state from raw z value
     * @param z Z-coordinate [0, 1]
     */
    void updateFromZ(float z);

    /**
     * @brief Get current phase state
     * @return Current phase state
     */
    const PhaseState& getState() const { return m_state; }

    /**
     * @brief Get current phase
     * @return Current Phase enum
     */
    Phase getCurrentPhase() const { return m_state.current; }

    /**
     * @brief Get current z-coordinate
     * @return z value [0, 1]
     */
    float getZ() const { return m_state.z; }

    /**
     * @brief Get smoothed z-coordinate
     * @return Smoothed z value
     */
    float getZSmoothed() const { return m_state.z_smoothed; }

    /**
     * @brief Get current tier (1-9)
     * @return Tier number
     */
    uint8_t getTier() const { return m_state.tier; }

    /**
     * @brief Check if a phase transition just occurred
     * @return true if transition in last update
     */
    bool hasTransition() const { return m_transition_flag; }

    /**
     * @brief Get the last phase transition
     * @return Last transition details
     */
    const PhaseTransition& getLastTransition() const { return m_last_transition; }

    /**
     * @brief Get frequency for current phase/tier
     * @return Solfeggio frequency in Hz
     */
    uint16_t getCurrentFrequency() const;

    /**
     * @brief Get RGB color for current phase
     * @param r Output red (0-255)
     * @param g Output green (0-255)
     * @param b Output blue (0-255)
     */
    void getCurrentColor(uint8_t& r, uint8_t& g, uint8_t& b) const;

    /**
     * @brief Set smoothing factor for z EMA
     * @param alpha Smoothing factor [0, 1], higher = less smoothing
     */
    void setSmoothingFactor(float alpha);

    /**
     * @brief Set phase stability threshold
     * @param ms Minimum time in phase to be "stable"
     */
    void setStabilityThreshold(uint32_t ms);

    /**
     * @brief Update LED indicators based on current phase
     */
    void updateIndicators();

    /**
     * @brief Get phase history for analysis
     * @param buffer Output buffer for history
     * @param maxEntries Maximum entries to return
     * @return Number of entries copied
     */
    uint16_t getHistory(PhaseHistoryEntry* buffer, uint16_t maxEntries);

private:
    /// Current phase state
    PhaseState m_state;

    /// Last transition details
    PhaseTransition m_last_transition;

    /// Transition flag (cleared each update)
    bool m_transition_flag;

    /// Smoothing factor for EMA
    float m_alpha;

    /// Stability threshold (ms)
    uint32_t m_stability_threshold;

    /// Previous z for velocity calculation
    float m_z_prev;

    /// Previous timestamp
    uint32_t m_time_prev;

    /// History buffer
    static const uint16_t HISTORY_SIZE = 256;
    PhaseHistoryEntry m_history[HISTORY_SIZE];
    uint16_t m_history_head;
    uint16_t m_history_count;

    /**
     * @brief Detect phase from z-coordinate with hysteresis
     * @param z Current z value
     * @return Detected phase
     */
    Phase detectPhase(float z);

    /**
     * @brief Record transition event
     * @param from Previous phase
     * @param to New phase
     * @param z Z at transition
     */
    void recordTransition(Phase from, Phase to, float z);

    /**
     * @brief Add entry to history buffer
     * @param z Z-coordinate
     * @param phase Current phase
     */
    void addToHistory(float z, Phase phase);
};

/**
 * @brief Convert phase enum to string
 * @param phase Phase value
 * @return String representation
 */
const char* phaseToString(Phase phase);

/**
 * @brief Get voltage threshold for phase boundary
 * @param boundary Phase boundary (0 = UNTRUE/PARADOX, 1 = PARADOX/TRUE)
 * @param vref Reference voltage (default 3.3V)
 * @return Threshold voltage
 */
float getPhaseVoltage(uint8_t boundary, float vref = 3.3f);

} // namespace UCF

#endif // PHASE_ENGINE_H

/**
 * @file triad_fsm.h
 * @brief TRIAD Unlock Finite State Machine
 *
 * Implements the hysteresis-based unlock system that requires
 * three threshold crossings to achieve TRIAD unlock state.
 * Maps to "bifurcation detection" in the conceptual framework.
 */

#ifndef TRIAD_FSM_H
#define TRIAD_FSM_H

#include <stdint.h>
#include "constants.h"

namespace UCF {

/// TRIAD FSM states
enum class TriadState : uint8_t {
    IDLE,           // Waiting for first crossing
    ARMED,          // Below low threshold, ready for rising
    CROSSING_1,     // First crossing detected
    CROSSING_2,     // Second crossing detected
    CROSSING_3,     // Third crossing detected
    UNLOCKED,       // TRIAD unlock achieved
    LOCKED_OUT      // Cooldown period after unlock
};

/// TRIAD event types
enum class TriadEvent : uint8_t {
    NONE,
    RISING_EDGE,    // Crossed above high threshold
    FALLING_EDGE,   // Crossed below low threshold
    UNLOCK,         // TRIAD unlocked
    TIMEOUT,        // Sequence timed out
    LOCKOUT_END     // Lockout period ended
};

/// TRIAD status structure
struct TriadStatus {
    TriadState state;           // Current FSM state
    uint8_t crossing_count;     // Number of crossings detected
    float current_value;        // Current input value
    bool is_unlocked;           // TRIAD unlock active
    uint32_t unlock_timestamp;  // When unlock occurred
    uint32_t state_duration;    // Time in current state (ms)
    uint32_t sequence_start;    // When sequence began
    TriadEvent last_event;      // Last event that occurred
};

/// TRIAD configuration
struct TriadConfig {
    float high_threshold;       // Rising edge threshold (default: 0.85)
    float low_threshold;        // Re-arm threshold (default: 0.82)
    float t6_gate;              // t6 unlock gate (default: 0.83)
    uint8_t passes_required;    // Crossings needed (default: 3)
    uint32_t sequence_timeout;  // Max time for sequence (ms)
    uint32_t lockout_duration;  // Cooldown after unlock (ms)
    uint32_t debounce_time;     // Minimum time between edges (ms)
};

/**
 * @class TriadFSM
 * @brief Hysteresis-based state machine for TRIAD unlock detection
 */
class TriadFSM {
public:
    /// Default constructor with UCF constants
    TriadFSM();

    /**
     * @brief Constructor with custom configuration
     * @param config TRIAD configuration
     */
    explicit TriadFSM(const TriadConfig& config);

    /**
     * @brief Initialize the FSM
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Update FSM with new input value
     * @param value Current coherence/z value [0, 1]
     * @return Event that occurred (if any)
     */
    TriadEvent update(float value);

    /**
     * @brief Reset the FSM to initial state
     */
    void reset();

    /**
     * @brief Force unlock (for testing/override)
     */
    void forceUnlock();

    /**
     * @brief Force lock (end unlock state)
     */
    void forceLock();

    /**
     * @brief Check if TRIAD is currently unlocked
     * @return true if unlocked
     */
    bool isUnlocked() const { return m_status.is_unlocked; }

    /**
     * @brief Get current state
     * @return Current FSM state
     */
    TriadState getState() const { return m_status.state; }

    /**
     * @brief Get full status
     * @return Current status structure
     */
    const TriadStatus& getStatus() const { return m_status; }

    /**
     * @brief Get crossing count
     * @return Number of crossings in current sequence
     */
    uint8_t getCrossingCount() const { return m_status.crossing_count; }

    /**
     * @brief Get configuration
     * @return Current configuration
     */
    const TriadConfig& getConfig() const { return m_config; }

    /**
     * @brief Set high threshold
     * @param threshold New high threshold
     */
    void setHighThreshold(float threshold);

    /**
     * @brief Set low threshold
     * @param threshold New low threshold
     */
    void setLowThreshold(float threshold);

    /**
     * @brief Get time since unlock (0 if not unlocked)
     * @return Milliseconds since unlock
     */
    uint32_t getUnlockDuration() const;

    /**
     * @brief Check if t6 gate is satisfied
     * @param z Current z-coordinate
     * @return true if z is at t6 gate level
     */
    bool checkT6Gate(float z) const;

    /**
     * @brief Update hardware indicator LED
     */
    void updateIndicator();

    /**
     * @brief Register callback for unlock event
     * @param callback Function to call on unlock
     */
    typedef void (*UnlockCallback)(void);
    void onUnlock(UnlockCallback callback);

private:
    /// Current status
    TriadStatus m_status;

    /// Configuration
    TriadConfig m_config;

    /// Previous value for edge detection
    float m_prev_value;

    /// Last edge timestamp (for debouncing)
    uint32_t m_last_edge_time;

    /// Unlock callback
    UnlockCallback m_unlock_callback;

    /**
     * @brief Transition to new state
     * @param new_state Target state
     */
    void transitionTo(TriadState new_state);

    /**
     * @brief Check for rising edge
     * @param value Current value
     * @return true if rising edge detected
     */
    bool detectRisingEdge(float value);

    /**
     * @brief Check for falling edge
     * @param value Current value
     * @return true if falling edge detected
     */
    bool detectFallingEdge(float value);

    /**
     * @brief Handle sequence timeout
     */
    void handleTimeout();

    /**
     * @brief Handle lockout expiry
     */
    void handleLockoutEnd();
};

/**
 * @brief Convert TRIAD state to string
 * @param state State value
 * @return String representation
 */
const char* triadStateToString(TriadState state);

/**
 * @brief Convert TRIAD event to string
 * @param event Event value
 * @return String representation
 */
const char* triadEventToString(TriadEvent event);

} // namespace UCF

#endif // TRIAD_FSM_H

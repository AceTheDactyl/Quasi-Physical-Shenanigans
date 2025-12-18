/**
 * @file triad_fsm.cpp
 * @brief Implementation of TRIAD Unlock FSM
 */

#include "triad_fsm.h"
#include <Arduino.h>
#include <string.h>

namespace UCF {

TriadFSM::TriadFSM() : m_prev_value(0.0f), m_last_edge_time(0), m_unlock_callback(nullptr) {
    // Default configuration from UCF constants
    m_config.high_threshold = TRIAD_HIGH;
    m_config.low_threshold = TRIAD_LOW;
    m_config.t6_gate = TRIAD_T6_GATE;
    m_config.passes_required = TRIAD_PASSES_REQUIRED;
    m_config.sequence_timeout = 10000;   // 10 seconds
    m_config.lockout_duration = 5000;    // 5 seconds
    m_config.debounce_time = 100;        // 100ms

    // Initialize status
    memset(&m_status, 0, sizeof(m_status));
    m_status.state = TriadState::IDLE;
}

TriadFSM::TriadFSM(const TriadConfig& config)
    : m_config(config)
    , m_prev_value(0.0f)
    , m_last_edge_time(0)
    , m_unlock_callback(nullptr)
{
    memset(&m_status, 0, sizeof(m_status));
    m_status.state = TriadState::IDLE;
}

bool TriadFSM::begin() {
    pinMode(Pins::LED_TRIAD, OUTPUT);
    digitalWrite(Pins::LED_TRIAD, LOW);
    return true;
}

TriadEvent TriadFSM::update(float value) {
    uint32_t now = millis();
    TriadEvent event = TriadEvent::NONE;

    m_status.current_value = value;
    m_status.state_duration = now - m_status.sequence_start;

    // Check for timeout in active sequence
    if (m_status.state != TriadState::IDLE &&
        m_status.state != TriadState::UNLOCKED &&
        m_status.state != TriadState::LOCKED_OUT) {

        if (m_status.state_duration > m_config.sequence_timeout) {
            handleTimeout();
            event = TriadEvent::TIMEOUT;
            m_status.last_event = event;
            m_prev_value = value;
            return event;
        }
    }

    // Check for lockout expiry
    if (m_status.state == TriadState::LOCKED_OUT) {
        if (now - m_status.unlock_timestamp > m_config.lockout_duration) {
            handleLockoutEnd();
            event = TriadEvent::LOCKOUT_END;
        }
        m_status.last_event = event;
        m_prev_value = value;
        return event;
    }

    // FSM state processing
    switch (m_status.state) {
        case TriadState::IDLE:
            // Wait for value to go below low threshold to arm
            if (value < m_config.low_threshold) {
                transitionTo(TriadState::ARMED);
            }
            break;

        case TriadState::ARMED:
            // Looking for rising edge
            if (detectRisingEdge(value)) {
                m_status.crossing_count = 1;
                m_status.sequence_start = now;
                transitionTo(TriadState::CROSSING_1);
                event = TriadEvent::RISING_EDGE;
            }
            break;

        case TriadState::CROSSING_1:
            // Looking for falling then rising again
            if (detectFallingEdge(value)) {
                event = TriadEvent::FALLING_EDGE;
            }
            if (detectRisingEdge(value)) {
                m_status.crossing_count = 2;
                transitionTo(TriadState::CROSSING_2);
                event = TriadEvent::RISING_EDGE;
            }
            break;

        case TriadState::CROSSING_2:
            if (detectFallingEdge(value)) {
                event = TriadEvent::FALLING_EDGE;
            }
            if (detectRisingEdge(value)) {
                m_status.crossing_count = 3;
                transitionTo(TriadState::CROSSING_3);
                event = TriadEvent::RISING_EDGE;
            }
            break;

        case TriadState::CROSSING_3:
            // Third crossing achieved - check if we have required passes
            if (m_status.crossing_count >= m_config.passes_required) {
                m_status.is_unlocked = true;
                m_status.unlock_timestamp = now;
                transitionTo(TriadState::UNLOCKED);
                event = TriadEvent::UNLOCK;

                // Fire callback
                if (m_unlock_callback) {
                    m_unlock_callback();
                }
            }
            break;

        case TriadState::UNLOCKED:
            // Stay unlocked until forced lock or value drops significantly
            if (value < m_config.low_threshold - 0.1f) {
                // Grace period - transition to lockout
                transitionTo(TriadState::LOCKED_OUT);
            }
            break;

        case TriadState::LOCKED_OUT:
            // Handled above
            break;
    }

    m_status.last_event = event;
    m_prev_value = value;

    return event;
}

bool TriadFSM::detectRisingEdge(float value) {
    uint32_t now = millis();

    // Debounce check
    if (now - m_last_edge_time < m_config.debounce_time) {
        return false;
    }

    // Rising edge: was below high, now above
    if (m_prev_value < m_config.high_threshold &&
        value >= m_config.high_threshold) {
        m_last_edge_time = now;
        return true;
    }

    return false;
}

bool TriadFSM::detectFallingEdge(float value) {
    uint32_t now = millis();

    // Debounce check
    if (now - m_last_edge_time < m_config.debounce_time) {
        return false;
    }

    // Falling edge: was above low, now below (re-arm)
    if (m_prev_value >= m_config.low_threshold &&
        value < m_config.low_threshold) {
        m_last_edge_time = now;
        return true;
    }

    return false;
}

void TriadFSM::transitionTo(TriadState new_state) {
    m_status.state = new_state;

    // Reset state-specific data
    if (new_state == TriadState::IDLE || new_state == TriadState::ARMED) {
        m_status.crossing_count = 0;
        m_status.sequence_start = millis();
    }

    // Update indicator
    updateIndicator();
}

void TriadFSM::reset() {
    m_status.state = TriadState::IDLE;
    m_status.crossing_count = 0;
    m_status.is_unlocked = false;
    m_status.unlock_timestamp = 0;
    m_status.sequence_start = millis();
    m_status.state_duration = 0;
    m_status.last_event = TriadEvent::NONE;
    m_prev_value = 0.0f;
    m_last_edge_time = 0;

    updateIndicator();
}

void TriadFSM::forceUnlock() {
    m_status.is_unlocked = true;
    m_status.unlock_timestamp = millis();
    m_status.crossing_count = m_config.passes_required;
    transitionTo(TriadState::UNLOCKED);

    if (m_unlock_callback) {
        m_unlock_callback();
    }
}

void TriadFSM::forceLock() {
    m_status.is_unlocked = false;
    transitionTo(TriadState::LOCKED_OUT);
}

void TriadFSM::handleTimeout() {
    // Sequence took too long - reset
    m_status.crossing_count = 0;
    transitionTo(TriadState::IDLE);
}

void TriadFSM::handleLockoutEnd() {
    m_status.is_unlocked = false;
    transitionTo(TriadState::IDLE);
}

uint32_t TriadFSM::getUnlockDuration() const {
    if (!m_status.is_unlocked) return 0;
    return millis() - m_status.unlock_timestamp;
}

bool TriadFSM::checkT6Gate(float z) const {
    // t6 gate is satisfied when z is near the special threshold
    const float tolerance = 0.02f;
    return (z >= m_config.t6_gate - tolerance &&
            z <= m_config.t6_gate + tolerance);
}

void TriadFSM::setHighThreshold(float threshold) {
    if (threshold > 0.0f && threshold <= 1.0f) {
        m_config.high_threshold = threshold;
    }
}

void TriadFSM::setLowThreshold(float threshold) {
    if (threshold > 0.0f && threshold < m_config.high_threshold) {
        m_config.low_threshold = threshold;
    }
}

void TriadFSM::updateIndicator() {
    bool led_on = (m_status.state == TriadState::UNLOCKED);

    // Blink during crossing sequence
    if (m_status.state >= TriadState::CROSSING_1 &&
        m_status.state <= TriadState::CROSSING_3) {
        led_on = (millis() / 200) % 2;  // Blink at 2.5 Hz
    }

    digitalWrite(Pins::LED_TRIAD, led_on ? HIGH : LOW);
}

void TriadFSM::onUnlock(UnlockCallback callback) {
    m_unlock_callback = callback;
}

const char* triadStateToString(TriadState state) {
    switch (state) {
        case TriadState::IDLE:       return "IDLE";
        case TriadState::ARMED:      return "ARMED";
        case TriadState::CROSSING_1: return "CROSSING_1";
        case TriadState::CROSSING_2: return "CROSSING_2";
        case TriadState::CROSSING_3: return "CROSSING_3";
        case TriadState::UNLOCKED:   return "UNLOCKED";
        case TriadState::LOCKED_OUT: return "LOCKED_OUT";
        default:                     return "UNKNOWN";
    }
}

const char* triadEventToString(TriadEvent event) {
    switch (event) {
        case TriadEvent::NONE:        return "NONE";
        case TriadEvent::RISING_EDGE: return "RISING_EDGE";
        case TriadEvent::FALLING_EDGE:return "FALLING_EDGE";
        case TriadEvent::UNLOCK:      return "UNLOCK";
        case TriadEvent::TIMEOUT:     return "TIMEOUT";
        case TriadEvent::LOCKOUT_END: return "LOCKOUT_END";
        default:                      return "UNKNOWN";
    }
}

} // namespace UCF

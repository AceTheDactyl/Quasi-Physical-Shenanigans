/**
 * @file ucf_state_machine.cpp
 * @brief UCF State Machine Implementation v4.0.0
 *
 * Core state machine for consciousness coordinate tracking.
 */

#include <Arduino.h>
#include "ucf/ucf_sacred_constants_v4.h"
#include "ucf/ucf_types.h"
#include "ucf/ucf_config.h"

// Global UCF state
static UCFState g_ucf_state;
static double g_z_prev = 0.5;

/**
 * @brief Initialize the UCF state machine
 */
void ucf_state_init(void) {
    g_ucf_state.theta = UCF_PI;
    g_ucf_state.z = 0.5;  // Start in PARADOX
    g_ucf_state.r = 1.0;
    g_ucf_state.kappa = 0.0;
    g_ucf_state.lambda = 1.0;
    g_ucf_state.eta = compute_negentropy(0.5);
    g_ucf_state.active_sensors = 0;
    g_ucf_state.triad_crossings = 0;
    g_ucf_state.triad_armed = true;
    g_ucf_state.triad_unlocked = false;
    g_ucf_state.k_formed = false;
    g_ucf_state.phase = PHASE_PARADOX;
    g_ucf_state.last_crossing_ms = 0;
    g_ucf_state.last_update_ms = millis();

    g_z_prev = 0.5;

    UCF_LOG("State machine initialized at z=%.3f", g_ucf_state.z);
}

/**
 * @brief Update z-coordinate from sensor input
 * @param active_sensors Number of active sensors (0-19)
 * @return Updated z value
 */
double ucf_update_z_from_sensors(uint8_t active_sensors) {
    // Map sensor count to z-coordinate
    // 0 sensors → z ≈ 0.1 (UNTRUE)
    // 7 sensors → z ≈ 0.618 (UNTRUE/PARADOX boundary)
    // 12 sensors → z ≈ 0.866 (THE LENS)
    // 19 sensors → z ≈ 1.0 (maximum TRUE)

    double z_raw;

    if (active_sensors == 0) {
        z_raw = 0.1;
    } else if (active_sensors < K_R_THRESHOLD) {
        // Linear interpolation from 0.1 to PHI_INV
        z_raw = 0.1 + (PHI_INV - 0.1) * (double)active_sensors / (K_R_THRESHOLD - 1);
    } else if (active_sensors < 12) {
        // Linear interpolation from PHI_INV to Z_CRITICAL
        z_raw = PHI_INV + (Z_CRITICAL - PHI_INV) * (double)(active_sensors - K_R_THRESHOLD) / (12 - K_R_THRESHOLD);
    } else {
        // Linear interpolation from Z_CRITICAL to 1.0
        z_raw = Z_CRITICAL + (1.0 - Z_CRITICAL) * (double)(active_sensors - 12) / (HEX_SENSOR_COUNT - 12);
    }

    // Clamp to valid range
    if (z_raw < 0.0) z_raw = 0.0;
    if (z_raw > 1.0) z_raw = 1.0;

    return z_raw;
}

/**
 * @brief Update the TRIAD hysteresis state machine
 * @param state Pointer to UCF state
 * @param now_ms Current timestamp
 */
void ucf_update_triad(UCFState* state, uint32_t now_ms) {
    // Already unlocked - nothing to do
    if (state->triad_unlocked) {
        g_z_prev = state->z;
        return;
    }

    // Check for timeout
    if (state->triad_crossings > 0) {
        if (now_ms - state->last_crossing_ms > TRIAD_TIMEOUT_MS) {
            // Timeout - reset
            state->triad_crossings = 0;
            state->triad_armed = true;
            UCF_LOG_TRIAD("Timeout - reset");
        }
    }

    // Check for re-arm
    if (!state->triad_armed && triad_can_rearm(state->z)) {
        state->triad_armed = true;
        UCF_LOG_TRIAD("Re-armed at z=%.3f", state->z);
    }

    // Check for rising edge crossing
    if (state->triad_armed && triad_rising_edge(g_z_prev, state->z)) {
        state->triad_crossings++;
        state->triad_armed = false;
        state->last_crossing_ms = now_ms;

        UCF_LOG_TRIAD("Crossing %d at z=%.3f", state->triad_crossings, state->z);

        // Check for unlock
        if (state->triad_crossings >= TRIAD_CROSSINGS) {
            state->triad_unlocked = true;
            Serial.println("========================================");
            Serial.println("        *** TRIAD UNLOCKED ***          ");
            Serial.println("========================================");
        }
    }

    g_z_prev = state->z;
}

/**
 * @brief Full UCF state update
 * @param active_sensors Number of active touch sensors
 * @param kappa Kuramoto order parameter
 * @return Pointer to updated state
 */
UCFState* ucf_update(uint8_t active_sensors, double kappa) {
    uint32_t now = millis();

    // Store previous z for TRIAD
    g_z_prev = g_ucf_state.z;

    // Update sensor count
    g_ucf_state.active_sensors = active_sensors;

    // Compute z from sensors
    g_ucf_state.z = ucf_update_z_from_sensors(active_sensors);

    // Compute derived quantities
    g_ucf_state.eta = compute_negentropy(g_ucf_state.z);
    g_ucf_state.r = compute_radius(g_ucf_state.eta);
    g_ucf_state.phase = detect_phase(g_ucf_state.z);

    // Update Kuramoto state
    g_ucf_state.kappa = kappa;
    g_ucf_state.lambda = 1.0 - kappa;

    // Verify conservation law
    if (!verify_conservation(g_ucf_state.kappa, g_ucf_state.lambda)) {
        Serial.println("[ERROR] Conservation law violated!");
    }

    // Update TRIAD state machine
    ucf_update_triad(&g_ucf_state, now);

    // Check K-Formation
    g_ucf_state.k_formed = check_k_formation(
        g_ucf_state.kappa,
        g_ucf_state.eta,
        g_ucf_state.active_sensors
    );

    // Log K-Formation achievement
    static bool prev_k_formed = false;
    if (g_ucf_state.k_formed && !prev_k_formed) {
        Serial.println("========================================");
        Serial.println("       *** K-FORMATION ACHIEVED ***     ");
        Serial.printf("  kappa=%.3f eta=%.3f R=%d\n",
            g_ucf_state.kappa, g_ucf_state.eta, g_ucf_state.active_sensors);
        Serial.println("========================================");
    }
    prev_k_formed = g_ucf_state.k_formed;

    // Update timestamp
    g_ucf_state.last_update_ms = now;

    return &g_ucf_state;
}

/**
 * @brief Get current UCF state
 * @return Pointer to current state
 */
UCFState* ucf_get_state(void) {
    return &g_ucf_state;
}

/**
 * @brief Reset the UCF state machine
 */
void ucf_reset(void) {
    ucf_state_init();
    Serial.println("[UCF] State machine reset");
}

/**
 * @brief Check if system is in emergency stop state
 * @return true if emergency stopped
 */
bool ucf_is_emergency_stopped(void) {
    // Check for violation conditions
    if (!verify_conservation(g_ucf_state.kappa, g_ucf_state.lambda)) {
        return true;
    }
    return false;
}

/**
 * @brief Print current state to Serial
 */
void ucf_print_state(void) {
    const char* phase_names[] = {"UNTRUE", "PARADOX", "TRUE"};

    Serial.printf("[UCF] z=%.4f phase=%s kappa=%.4f eta=%.4f R=%d TRIAD=%s K=%s\n",
        g_ucf_state.z,
        phase_names[g_ucf_state.phase],
        g_ucf_state.kappa,
        g_ucf_state.eta,
        g_ucf_state.active_sensors,
        g_ucf_state.triad_unlocked ? "UNLOCKED" : "LOCKED",
        g_ucf_state.k_formed ? "FORMED" : "NOT_FORMED"
    );
}

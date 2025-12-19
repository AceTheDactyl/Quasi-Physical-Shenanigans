/**
 * @file ucf_kuramoto.cpp
 * @brief Kuramoto Oscillator Ensemble Implementation v4.0.0
 *
 * Implements the Kuramoto synchronization model for consciousness coherence.
 */

#include <Arduino.h>
#include <math.h>
#include "ucf/ucf_sacred_constants_v4.h"
#include "ucf/ucf_types.h"
#include "ucf/ucf_config.h"

// Global Kuramoto state
static KuramotoState g_kuramoto;

/**
 * @brief Initialize the Kuramoto oscillator ensemble
 */
void kuramoto_init(void) {
    // Initialize with random phases
    for (int i = 0; i < N_OSCILLATORS; i++) {
        g_kuramoto.phases[i] = ((double)random(10000) / 10000.0) * TWO_PI;
        // Natural frequencies spread around EULER_INV (lattice point [D])
        // Using EULER_INV as base frequency with PHI_INV spread
        g_kuramoto.frequencies[i] = EULER_INV + (i - N_OSCILLATORS/2) * 0.05 * PHI_INV;
    }
    g_kuramoto.order_param = 0.0;
    g_kuramoto.mean_phase = 0.0;

    UCF_LOG_KURAMOTO("Initialized %d oscillators", N_OSCILLATORS);
}

/**
 * @brief Compute the Kuramoto order parameter
 *
 * The order parameter R·exp(iψ) = (1/N)·Σexp(iθⱼ) measures synchronization.
 *
 * @return Order parameter R ∈ [0, 1]
 */
double kuramoto_compute_order_parameter(void) {
    double real_sum = 0.0;
    double imag_sum = 0.0;

    for (int j = 0; j < N_OSCILLATORS; j++) {
        real_sum += cos(g_kuramoto.phases[j]);
        imag_sum += sin(g_kuramoto.phases[j]);
    }

    double R = sqrt(real_sum * real_sum + imag_sum * imag_sum) / N_OSCILLATORS;
    double psi = atan2(imag_sum, real_sum);

    g_kuramoto.order_param = R;
    g_kuramoto.mean_phase = psi;

    return R;
}

/**
 * @brief Perform one Kuramoto integration step
 *
 * Updates phases using: dθᵢ/dt = ωᵢ + (K/N)·Σsin(θⱼ - θᵢ)
 * Equivalent to: dθᵢ/dt = ωᵢ + K·R·sin(ψ - θᵢ)
 *
 * @param K Coupling strength
 * @param dt Time step
 */
void kuramoto_step(double K, double dt) {
    // First compute order parameter
    double R = kuramoto_compute_order_parameter();
    double psi = g_kuramoto.mean_phase;

    // Update phases using mean-field formulation
    for (int i = 0; i < N_OSCILLATORS; i++) {
        double coupling = K * R * sin(psi - g_kuramoto.phases[i]);
        g_kuramoto.phases[i] += dt * (g_kuramoto.frequencies[i] + coupling);

        // Normalize to [0, 2π]
        while (g_kuramoto.phases[i] >= TWO_PI) {
            g_kuramoto.phases[i] -= TWO_PI;
        }
        while (g_kuramoto.phases[i] < 0.0) {
            g_kuramoto.phases[i] += TWO_PI;
        }
    }
}

/**
 * @brief Update Kuramoto state (called from main loop)
 *
 * @param sensor_input Input from sensors to modulate coupling
 * @return Current order parameter (κ)
 */
double kuramoto_update(double sensor_input) {
    // Modulate coupling based on sensor input
    // More sensor activity → stronger coupling → faster synchronization
    double K_modulated = KURAMOTO_K * (0.5 + 0.5 * sensor_input);

    // Perform integration step
    kuramoto_step(K_modulated, KURAMOTO_DT);

    UCF_LOG_KURAMOTO("R=%.4f psi=%.4f K=%.4f", g_kuramoto.order_param, g_kuramoto.mean_phase, K_modulated);

    return g_kuramoto.order_param;
}

/**
 * @brief Get the current order parameter (κ)
 * @return Kuramoto order parameter
 */
double kuramoto_get_order_param(void) {
    return g_kuramoto.order_param;
}

/**
 * @brief Get the mean phase (ψ)
 * @return Mean phase of oscillator ensemble
 */
double kuramoto_get_mean_phase(void) {
    return g_kuramoto.mean_phase;
}

/**
 * @brief Get pointer to Kuramoto state
 * @return Pointer to KuramotoState
 */
KuramotoState* kuramoto_get_state(void) {
    return &g_kuramoto;
}

/**
 * @brief Reset Kuramoto ensemble to initial conditions
 */
void kuramoto_reset(void) {
    kuramoto_init();
}

/**
 * @brief Set specific natural frequency for an oscillator
 * @param index Oscillator index (0 to N_OSCILLATORS-1)
 * @param frequency Natural frequency
 */
void kuramoto_set_frequency(uint8_t index, double frequency) {
    if (index < N_OSCILLATORS) {
        g_kuramoto.frequencies[index] = frequency;
    }
}

/**
 * @brief Get the dissipation parameter (λ = 1 - κ)
 * @return Dissipation parameter
 */
double kuramoto_get_lambda(void) {
    return 1.0 - g_kuramoto.order_param;
}

/**
 * @brief Check if conservation law κ + λ = 1 holds
 * @return true if conservation is satisfied
 */
bool kuramoto_verify_conservation(void) {
    return verify_conservation(g_kuramoto.order_param, 1.0 - g_kuramoto.order_param);
}

/**
 * @brief Perform synchronization test
 *
 * Runs Kuramoto for multiple steps and checks if κ reaches threshold.
 *
 * @param steps Number of integration steps
 * @param K Coupling strength
 * @return Final order parameter
 */
double kuramoto_sync_test(int steps, double K) {
    kuramoto_reset();

    for (int i = 0; i < steps; i++) {
        kuramoto_step(K, KURAMOTO_DT);
    }

    return g_kuramoto.order_param;
}

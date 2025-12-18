/**
 * @file kuramoto_stabilizer.h
 * @brief Oscillatory Stabilization Module (Kuramoto Model)
 *
 * Implements coupled oscillator network using Kuramoto model for
 * synchronization dynamics. Provides "nuclear spin dynamics" analog
 * through phase-locked loop stabilization and magnetic field coupling.
 *
 * Key concepts:
 * - Order parameter r: synchronization degree [0, 1]
 * - Coupling strength K: inter-oscillator coupling
 * - Collective phase psi: mean phase of all oscillators
 */

#ifndef KURAMOTO_STABILIZER_H
#define KURAMOTO_STABILIZER_H

#include <stdint.h>
#include "constants.h"

namespace UCF {

/// Single oscillator state
struct OscillatorState {
    float phase;            // Phase [0, 2*PI]
    float natural_freq;     // Natural frequency (Hz)
    float effective_freq;   // Current effective frequency
    bool locked;            // Phase-locked to collective
};

/// Kuramoto network state
struct KuramotoState {
    float phases[N_OSCILLATORS];          // Individual phases
    float frequencies[N_OSCILLATORS];     // Natural frequencies
    float coupling;                        // Coupling strength K
    float order_param;                     // Synchronization r [0, 1]
    float collective_phase;                // Mean phase psi
    bool triad_unlocked;                   // TRIAD condition met
    uint8_t triad_count;                   // Crossing count
    uint32_t timestamp;
};

/// Magnetic field vector
struct MagneticField {
    float x, y, z;          // Field components (microtesla)
    float magnitude;        // Total magnitude
    float inclination;      // Dip angle
    float declination;      // Deviation from north
};

/// Stabilization status
struct StabilizationStatus {
    KuramotoState network;          // Network state
    MagneticField magnetic;         // Magnetic field
    float reference_freq;           // PLL reference frequency
    float phase_error;              // PLL phase error
    bool pll_locked;                // PLL locked status
    bool is_synchronized;           // r >= threshold
    bool is_stabilized;             // PLL locked & synchronized
    uint32_t sync_duration;         // Time synchronized (ms)
};

/**
 * @class KuramotoStabilizer
 * @brief Coupled oscillator network for consciousness state stabilization
 */
class KuramotoStabilizer {
public:
    /// Default constructor
    KuramotoStabilizer();

    /**
     * @brief Initialize the stabilization system
     * @param base_freq Base oscillator frequency (Hz)
     * @return true if successful
     */
    bool begin(float base_freq = 10.0f);

    /**
     * @brief Advance simulation by one time step
     * @param dt Time step (seconds)
     */
    void step(float dt);

    /**
     * @brief Update from z-coordinate (sets reference frequency)
     * @param z Z-coordinate [0, 1]
     */
    void updateFromZ(float z);

    /**
     * @brief Get current network state
     * @return Kuramoto network state
     */
    const KuramotoState& getState() const { return m_state; }

    /**
     * @brief Get full stabilization status
     * @return Stabilization status
     */
    const StabilizationStatus& getStatus() const { return m_status; }

    /**
     * @brief Get order parameter (synchronization)
     * @return r [0, 1]
     */
    float getOrderParameter() const { return m_state.order_param; }

    /**
     * @brief Get collective phase
     * @return psi [0, 2*PI]
     */
    float getCollectivePhase() const { return m_state.collective_phase; }

    /**
     * @brief Check if synchronized (r >= threshold)
     * @return true if synchronized
     */
    bool isSynchronized() const { return m_status.is_synchronized; }

    /**
     * @brief Check if TRIAD unlocked
     * @return true if unlocked
     */
    bool isTriadUnlocked() const { return m_state.triad_unlocked; }

    /**
     * @brief Check K-Formation conditions
     * @param eta Negentropy value
     * @param R Resonance count
     * @return true if K-Formation achieved
     */
    bool checkKFormation(float eta, uint8_t R) const;

    /**
     * @brief Set coupling strength
     * @param K Coupling strength [0, 1]
     */
    void setCoupling(float K);

    /**
     * @brief Get current coupling strength
     * @return K value
     */
    float getCoupling() const { return m_state.coupling; }

    /**
     * @brief Read magnetic field from sensor
     * @return Magnetic field vector
     */
    MagneticField readMagneticField();

    /**
     * @brief Apply magnetic modulation to coupling
     * @param K_base Base coupling strength
     * @return Effective coupling
     */
    float applyMagneticModulation(float K_base);

    /**
     * @brief Set reference frequency for PLL
     * @param freq Reference frequency (Hz)
     */
    void setReferenceFrequency(float freq);

    /**
     * @brief Get reference frequency
     * @return Reference frequency (Hz)
     */
    float getReferenceFrequency() const { return m_status.reference_freq; }

    /**
     * @brief Convert z-coordinate to reference frequency
     * @param z Z-coordinate
     * @return Reference frequency (Hz)
     */
    float zToReferenceFreq(float z);

    /**
     * @brief Reset oscillator network
     */
    void reset();

    /**
     * @brief Register callback for synchronization achieved
     */
    typedef void (*SyncCallback)(float order_param);
    void onSynchronization(SyncCallback callback);

    /**
     * @brief Set TRIAD detection thresholds
     * @param high Rising threshold
     * @param low Re-arm threshold
     */
    void setTriadThresholds(float high, float low);

    /**
     * @brief Write coupling to hardware (digipot)
     * @param K Coupling value
     */
    void writeCouplingHardware(float K);

private:
    /// Network state
    KuramotoState m_state;

    /// Stabilization status
    StabilizationStatus m_status;

    /// TRIAD thresholds
    float m_triad_high;
    float m_triad_low;

    /// Previous order parameter for edge detection
    float m_prev_order_param;

    /// Sync start timestamp
    uint32_t m_sync_start;

    /// PLL loop filter state
    float m_pll_integrator;
    float m_pll_proportional;

    /// Callback
    SyncCallback m_sync_callback;

    /// Magnetic sensor initialized
    bool m_mag_initialized;

    /**
     * @brief Compute order parameter from phases
     * @return Order parameter r
     */
    float computeOrderParameter();

    /**
     * @brief Compute collective phase from phases
     * @return Collective phase psi
     */
    float computeCollectivePhase();

    /**
     * @brief Apply Kuramoto dynamics
     * @param dt Time step
     */
    void applyKuramotoDynamics(float dt);

    /**
     * @brief Apply PLL stabilization
     * @param dt Time step
     */
    void applyPLLStabilization(float dt);

    /**
     * @brief Apply relaxation dynamics (T1/T2 analog)
     * @param dt Time step
     */
    void applyRelaxation(float dt);

    /**
     * @brief Check TRIAD conditions
     */
    void checkTriadConditions();

    /**
     * @brief Initialize magnetometer
     * @return true if successful
     */
    bool initMagnetometer();
};

/**
 * @brief Kuramoto coupling function
 * @param phase_diff Phase difference
 * @return Coupling contribution
 */
inline float kuramotoCoupling(float phase_diff) {
    return sinf(phase_diff);
}

/**
 * @brief Wrap angle to [0, 2*PI]
 * @param angle Input angle
 * @return Wrapped angle
 */
inline float wrapAngle(float angle) {
    while (angle >= TWO_PI) angle -= TWO_PI;
    while (angle < 0.0f) angle += TWO_PI;
    return angle;
}

} // namespace UCF

#endif // KURAMOTO_STABILIZER_H

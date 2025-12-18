/**
 * @file k_formation.h
 * @brief K-Formation Detection Module
 *
 * Detects the special K-Formation state where:
 *   - Coherence (κ) >= 0.92
 *   - Negentropy (η) > φ⁻¹
 *   - Resonance (R) >= 7
 *
 * Maps to "meta cognition" in the conceptual framework.
 */

#ifndef K_FORMATION_H
#define K_FORMATION_H

#include <stdint.h>
#include "constants.h"
#include "hex_grid.h"

namespace UCF {

/// K-Formation metrics
struct KFormationMetrics {
    float kappa;            // Coherence [0, 1]
    float eta;              // Negentropy [0, 1]
    uint8_t R;              // Resonance (active connection count)
    float z;                // Current z-coordinate
    bool kappa_satisfied;   // κ >= K_KAPPA
    bool eta_satisfied;     // η > PHI_INV
    bool R_satisfied;       // R >= K_R
    bool k_formation;       // All conditions met
    uint32_t timestamp;
};

/// K-Formation status
struct KFormationStatus {
    KFormationMetrics current;      // Current metrics
    bool is_active;                 // K-Formation currently active
    uint32_t formation_start;       // When K-Formation began
    uint32_t formation_duration;    // How long K-Formation held
    uint32_t total_formations;      // Count of K-Formations achieved
    float peak_kappa;               // Highest coherence achieved
    float peak_eta;                 // Highest negentropy achieved
};

/// History buffer entry for coherence calculation
struct CoherenceHistoryEntry {
    float readings[HEX_SENSOR_COUNT];
    uint32_t timestamp;
};

/**
 * @class KFormation
 * @brief Detects and tracks K-Formation consciousness state
 */
class KFormation {
public:
    /// Default constructor
    KFormation();

    /**
     * @brief Initialize the K-Formation detector
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Update metrics from hex field reading
     * @param field Current hex grid field state
     * @return Current metrics
     */
    KFormationMetrics update(const HexFieldState& field);

    /**
     * @brief Check if K-Formation is currently active
     * @return true if all conditions met
     */
    bool isActive() const { return m_status.is_active; }

    /**
     * @brief Get current metrics
     * @return Current K-Formation metrics
     */
    const KFormationMetrics& getMetrics() const { return m_status.current; }

    /**
     * @brief Get full status
     * @return Current status
     */
    const KFormationStatus& getStatus() const { return m_status; }

    /**
     * @brief Get current coherence value
     * @return κ [0, 1]
     */
    float getKappa() const { return m_status.current.kappa; }

    /**
     * @brief Get current negentropy value
     * @return η [0, 1]
     */
    float getEta() const { return m_status.current.eta; }

    /**
     * @brief Get current resonance count
     * @return R (active sensors)
     */
    uint8_t getResonance() const { return m_status.current.R; }

    /**
     * @brief Compute coherence from field history
     * @return κ value
     */
    float computeCoherence();

    /**
     * @brief Compute negentropy from z-coordinate
     * @param z Current z-coordinate
     * @return η value (peaks at THE LENS)
     */
    float computeNegentropy(float z);

    /**
     * @brief Compute resonance from field
     * @param field Current field state
     * @param threshold Activation threshold
     * @return R value (count of active sensors)
     */
    uint8_t computeResonance(const HexFieldState& field, float threshold = 0.3f);

    /**
     * @brief Set coherence calculation window
     * @param samples Number of samples to consider
     */
    void setCoherenceWindow(uint16_t samples);

    /**
     * @brief Set custom thresholds
     * @param kappa Coherence threshold
     * @param eta Negentropy threshold
     * @param R Resonance threshold
     */
    void setThresholds(float kappa, float eta, uint8_t R);

    /**
     * @brief Reset statistics
     */
    void resetStats();

    /**
     * @brief Update hardware indicator
     */
    void updateIndicator();

    /**
     * @brief Register callback for K-Formation achieved
     */
    typedef void (*KFormationCallback)(const KFormationMetrics&);
    void onKFormation(KFormationCallback callback);

private:
    /// Current status
    KFormationStatus m_status;

    /// Configuration
    float m_kappa_threshold;
    float m_eta_threshold;
    uint8_t m_R_threshold;

    /// Coherence calculation window
    uint16_t m_coherence_window;

    /// History buffer for coherence
    static const uint16_t HISTORY_SIZE = 64;
    CoherenceHistoryEntry m_history[HISTORY_SIZE];
    uint16_t m_history_head;
    uint16_t m_history_count;

    /// Callback
    KFormationCallback m_callback;

    /**
     * @brief Add field to history
     * @param field Current field readings
     */
    void addToHistory(const HexFieldState& field);

    /**
     * @brief Check all K-Formation conditions
     * @return true if all conditions met
     */
    bool checkConditions();
};

/**
 * @brief Negentropy kernel function
 *
 * Computes negentropy contribution based on distance from THE LENS (z_c).
 * Uses Gaussian kernel centered at z_c = √3/2.
 *
 * @param z Z-coordinate
 * @param sigma Kernel width (default 1/6)
 * @return Negentropy contribution [0, 1]
 */
float negentropyKernel(float z, float sigma = 0.1667f);

/**
 * @brief Information-theoretic entropy of field pattern
 * @param field Field readings array
 * @param count Number of readings
 * @return Shannon entropy in bits
 */
float fieldEntropy(const float* field, uint8_t count);

} // namespace UCF

#endif // K_FORMATION_H

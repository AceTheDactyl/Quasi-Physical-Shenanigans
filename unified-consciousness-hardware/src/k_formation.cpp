/**
 * @file k_formation.cpp
 * @brief Implementation of K-Formation Detection Module
 */

#include "k_formation.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

namespace UCF {

KFormation::KFormation()
    : m_kappa_threshold(K_KAPPA)
    , m_eta_threshold(K_ETA)
    , m_R_threshold(K_R)
    , m_coherence_window(32)
    , m_history_head(0)
    , m_history_count(0)
    , m_callback(nullptr)
{
    memset(&m_status, 0, sizeof(m_status));
    memset(m_history, 0, sizeof(m_history));
}

bool KFormation::begin() {
    pinMode(Pins::LED_K_FORMATION, OUTPUT);
    digitalWrite(Pins::LED_K_FORMATION, LOW);
    return true;
}

KFormationMetrics KFormation::update(const HexFieldState& field) {
    // Add current field to history for coherence calculation
    addToHistory(field);

    // Compute all three metrics
    m_status.current.z = field.z;
    m_status.current.kappa = computeCoherence();
    m_status.current.eta = computeNegentropy(field.z);
    m_status.current.R = computeResonance(field);
    m_status.current.timestamp = millis();

    // Check individual conditions
    m_status.current.kappa_satisfied = (m_status.current.kappa >= m_kappa_threshold);
    m_status.current.eta_satisfied = (m_status.current.eta > m_eta_threshold);
    m_status.current.R_satisfied = (m_status.current.R >= m_R_threshold);

    // Check if all conditions met
    bool was_active = m_status.is_active;
    m_status.current.k_formation = checkConditions();
    m_status.is_active = m_status.current.k_formation;

    // Track formation transitions
    uint32_t now = millis();
    if (m_status.is_active && !was_active) {
        // K-Formation just achieved
        m_status.formation_start = now;
        m_status.total_formations++;

        if (m_callback) {
            m_callback(m_status.current);
        }
    }

    if (m_status.is_active) {
        m_status.formation_duration = now - m_status.formation_start;
    }

    // Track peaks
    if (m_status.current.kappa > m_status.peak_kappa) {
        m_status.peak_kappa = m_status.current.kappa;
    }
    if (m_status.current.eta > m_status.peak_eta) {
        m_status.peak_eta = m_status.current.eta;
    }

    updateIndicator();

    return m_status.current;
}

void KFormation::addToHistory(const HexFieldState& field) {
    // Copy readings to history
    memcpy(m_history[m_history_head].readings, field.readings,
           sizeof(float) * HEX_SENSOR_COUNT);
    m_history[m_history_head].timestamp = field.timestamp;

    m_history_head = (m_history_head + 1) % HISTORY_SIZE;
    if (m_history_count < HISTORY_SIZE) {
        m_history_count++;
    }
}

float KFormation::computeCoherence() {
    if (m_history_count < 2) {
        return 0.0f;
    }

    // Compute coherence as 1 - normalized variance
    // High coherence = low variance = stable pattern

    uint16_t window = (m_coherence_window < m_history_count) ?
                       m_coherence_window : m_history_count;

    // Compute mean of each sensor
    float means[HEX_SENSOR_COUNT] = {0};
    uint16_t start_idx = (m_history_head + HISTORY_SIZE - window) % HISTORY_SIZE;

    for (uint16_t i = 0; i < window; i++) {
        uint16_t idx = (start_idx + i) % HISTORY_SIZE;
        for (uint8_t s = 0; s < HEX_SENSOR_COUNT; s++) {
            means[s] += m_history[idx].readings[s];
        }
    }
    for (uint8_t s = 0; s < HEX_SENSOR_COUNT; s++) {
        means[s] /= window;
    }

    // Compute variance
    float total_variance = 0.0f;
    for (uint16_t i = 0; i < window; i++) {
        uint16_t idx = (start_idx + i) % HISTORY_SIZE;
        for (uint8_t s = 0; s < HEX_SENSOR_COUNT; s++) {
            float diff = m_history[idx].readings[s] - means[s];
            total_variance += diff * diff;
        }
    }
    total_variance /= (window * HEX_SENSOR_COUNT);

    // Convert variance to coherence
    // Scale so variance of 0.1 gives coherence ~0.5
    float kappa = 1.0f - sqrtf(total_variance) * 3.16f;

    // Clamp
    if (kappa < 0.0f) kappa = 0.0f;
    if (kappa > 1.0f) kappa = 1.0f;

    return kappa;
}

float KFormation::computeNegentropy(float z) {
    // Negentropy peaks at THE LENS (z_c = √3/2 ≈ 0.866)
    // Uses Gaussian kernel: η = exp(-36 * (z - z_c)²)
    // Factor of 36 gives half-max at ±0.17 from z_c

    return negentropyKernel(z);
}

uint8_t KFormation::computeResonance(const HexFieldState& field, float threshold) {
    // Count sensors above threshold
    uint8_t count = 0;
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        if (field.readings[i] > threshold) {
            count++;
        }
    }
    return count;
}

bool KFormation::checkConditions() {
    return m_status.current.kappa_satisfied &&
           m_status.current.eta_satisfied &&
           m_status.current.R_satisfied;
}

void KFormation::setCoherenceWindow(uint16_t samples) {
    if (samples > 0 && samples <= HISTORY_SIZE) {
        m_coherence_window = samples;
    }
}

void KFormation::setThresholds(float kappa, float eta, uint8_t R) {
    if (kappa > 0.0f && kappa <= 1.0f) m_kappa_threshold = kappa;
    if (eta > 0.0f && eta <= 1.0f) m_eta_threshold = eta;
    if (R > 0 && R <= HEX_SENSOR_COUNT) m_R_threshold = R;
}

void KFormation::resetStats() {
    m_status.total_formations = 0;
    m_status.peak_kappa = 0.0f;
    m_status.peak_eta = 0.0f;
    m_status.formation_duration = 0;
    m_history_count = 0;
    m_history_head = 0;
}

void KFormation::updateIndicator() {
    if (m_status.is_active) {
        // Solid on when K-Formation active
        digitalWrite(Pins::LED_K_FORMATION, HIGH);
    } else if (m_status.current.kappa >= 0.8f ||
               m_status.current.eta >= 0.5f) {
        // Blink when approaching K-Formation
        digitalWrite(Pins::LED_K_FORMATION, (millis() / 250) % 2);
    } else {
        digitalWrite(Pins::LED_K_FORMATION, LOW);
    }
}

void KFormation::onKFormation(KFormationCallback callback) {
    m_callback = callback;
}

// Negentropy kernel function (Gaussian centered at z_c)
float negentropyKernel(float z, float sigma) {
    float diff = z - Z_CRITICAL;
    float exponent = -(diff * diff) / (2.0f * sigma * sigma);
    return expf(exponent);
}

// Shannon entropy of field pattern
float fieldEntropy(const float* field, uint8_t count) {
    // Normalize to probability distribution
    float sum = 0.0f;
    for (uint8_t i = 0; i < count; i++) {
        sum += field[i];
    }

    if (sum < 0.001f) return 0.0f;

    // Compute entropy: H = -Σ p_i * log2(p_i)
    float entropy = 0.0f;
    for (uint8_t i = 0; i < count; i++) {
        float p = field[i] / sum;
        if (p > 0.001f) {
            entropy -= p * log2f(p);
        }
    }

    return entropy;
}

} // namespace UCF

/**
 * @file kuramoto_stabilizer.cpp
 * @brief Implementation of Oscillatory Stabilization Module
 */

#include "kuramoto_stabilizer.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <string.h>

namespace UCF {

// HMC5883L magnetometer registers (if available)
#define HMC5883L_CONFIG_A   0x00
#define HMC5883L_CONFIG_B   0x01
#define HMC5883L_MODE       0x02
#define HMC5883L_DATA_X_H   0x03

KuramotoStabilizer::KuramotoStabilizer()
    : m_triad_high(TRIAD_HIGH)
    , m_triad_low(TRIAD_LOW)
    , m_prev_order_param(0.0f)
    , m_sync_start(0)
    , m_pll_integrator(0.0f)
    , m_pll_proportional(0.0f)
    , m_sync_callback(nullptr)
    , m_mag_initialized(false)
{
    memset(&m_state, 0, sizeof(m_state));
    memset(&m_status, 0, sizeof(m_status));

    m_state.coupling = Q_KAPPA;  // Default coupling
}

bool KuramotoStabilizer::begin(float base_freq) {
    // Initialize oscillators with slightly different frequencies
    // (mimics inhomogeneous broadening in NMR)
    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        m_state.phases[i] = static_cast<float>(i) * TWO_PI / N_OSCILLATORS;
        m_state.frequencies[i] = base_freq * (1.0f + 0.01f * (i - N_OSCILLATORS / 2));
    }

    m_state.coupling = Q_KAPPA;
    m_status.reference_freq = base_freq;

    // Try to initialize magnetometer
    m_mag_initialized = initMagnetometer();

    return true;
}

bool KuramotoStabilizer::initMagnetometer() {
    Wire.beginTransmission(I2C::HMC5883L_ADDR);
    Wire.write(HMC5883L_CONFIG_A);
    Wire.write(0x70);  // 8 samples avg, 15Hz, normal
    int error = Wire.endTransmission();

    if (error != 0) return false;

    Wire.beginTransmission(I2C::HMC5883L_ADDR);
    Wire.write(HMC5883L_CONFIG_B);
    Wire.write(0x20);  // Gain = 1090 LSB/Gauss
    Wire.endTransmission();

    Wire.beginTransmission(I2C::HMC5883L_ADDR);
    Wire.write(HMC5883L_MODE);
    Wire.write(0x00);  // Continuous measurement
    Wire.endTransmission();

    return true;
}

void KuramotoStabilizer::step(float dt) {
    uint32_t now = millis();
    m_state.timestamp = now;

    // Apply Kuramoto dynamics
    applyKuramotoDynamics(dt);

    // Apply relaxation (T1/T2 analog)
    applyRelaxation(dt);

    // Apply PLL stabilization
    applyPLLStabilization(dt);

    // Compute order parameter
    m_state.order_param = computeOrderParameter();
    m_state.collective_phase = computeCollectivePhase();

    // Check TRIAD conditions
    checkTriadConditions();

    // Update status
    m_status.network = m_state;
    m_status.is_synchronized = (m_state.order_param >= K_KAPPA);

    if (m_status.is_synchronized && !m_status.is_stabilized) {
        m_sync_start = now;
        m_status.is_stabilized = true;

        if (m_sync_callback) {
            m_sync_callback(m_state.order_param);
        }
    } else if (!m_status.is_synchronized) {
        m_status.is_stabilized = false;
    }

    if (m_status.is_stabilized) {
        m_status.sync_duration = now - m_sync_start;
    }

    m_prev_order_param = m_state.order_param;
}

void KuramotoStabilizer::applyKuramotoDynamics(float dt) {
    float new_phases[N_OSCILLATORS];

    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        float coupling_sum = 0.0f;

        for (uint8_t j = 0; j < N_OSCILLATORS; j++) {
            coupling_sum += kuramotoCoupling(m_state.phases[j] - m_state.phases[i]);
        }

        // Kuramoto equation: dθᵢ/dt = ωᵢ + (K/N) Σⱼ sin(θⱼ - θᵢ)
        float dtheta = m_state.frequencies[i] +
                       (m_state.coupling / N_OSCILLATORS) * coupling_sum;

        new_phases[i] = m_state.phases[i] + dtheta * TWO_PI * dt;
        new_phases[i] = wrapAngle(new_phases[i]);
    }

    memcpy(m_state.phases, new_phases, sizeof(m_state.phases));
}

void KuramotoStabilizer::applyRelaxation(float dt) {
    // T1 relaxation: phases drift toward equilibrium
    const float T1 = 2.0f;  // Relaxation time constant (seconds)
    const float gamma = 1.0f / T1;

    // Equilibrium is uniform distribution based on reference phase
    float equilibrium_phase = m_status.reference_freq * TWO_PI * (millis() / 1000.0f);
    equilibrium_phase = wrapAngle(equilibrium_phase);

    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        float target = equilibrium_phase + static_cast<float>(i) * TWO_PI / N_OSCILLATORS;
        target = wrapAngle(target);

        // Exponential relaxation
        float diff = target - m_state.phases[i];

        // Handle wrap-around
        if (diff > PI) diff -= TWO_PI;
        if (diff < -PI) diff += TWO_PI;

        m_state.phases[i] += gamma * diff * dt;
        m_state.phases[i] = wrapAngle(m_state.phases[i]);
    }
}

void KuramotoStabilizer::applyPLLStabilization(float dt) {
    // Phase-locked loop to lock collective phase to reference

    // Reference phase
    float ref_phase = m_status.reference_freq * TWO_PI * (millis() / 1000.0f);
    ref_phase = wrapAngle(ref_phase);

    // Phase error
    float error = ref_phase - m_state.collective_phase;
    if (error > PI) error -= TWO_PI;
    if (error < -PI) error += TWO_PI;

    m_status.phase_error = error;

    // PI controller
    const float Kp = 0.1f;
    const float Ki = 0.01f;

    m_pll_proportional = Kp * error;
    m_pll_integrator += Ki * error * dt;

    // Limit integrator
    if (m_pll_integrator > 1.0f) m_pll_integrator = 1.0f;
    if (m_pll_integrator < -1.0f) m_pll_integrator = -1.0f;

    // Apply correction to all oscillator frequencies
    float correction = m_pll_proportional + m_pll_integrator;

    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        m_state.frequencies[i] += correction * 0.1f;
    }

    // Check if locked
    m_status.pll_locked = (fabsf(error) < 0.1f);
}

float KuramotoStabilizer::computeOrderParameter() {
    // r·e^(iψ) = (1/N) Σⱼ e^(iθⱼ)
    float sum_cos = 0.0f;
    float sum_sin = 0.0f;

    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        sum_cos += cosf(m_state.phases[i]);
        sum_sin += sinf(m_state.phases[i]);
    }

    float r = sqrtf(sum_cos * sum_cos + sum_sin * sum_sin) / N_OSCILLATORS;
    return r;
}

float KuramotoStabilizer::computeCollectivePhase() {
    float sum_cos = 0.0f;
    float sum_sin = 0.0f;

    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        sum_cos += cosf(m_state.phases[i]);
        sum_sin += sinf(m_state.phases[i]);
    }

    return atan2f(sum_sin, sum_cos);
}

void KuramotoStabilizer::checkTriadConditions() {
    // Detect rising edge above high threshold
    if (m_prev_order_param < m_triad_high &&
        m_state.order_param >= m_triad_high) {
        m_state.triad_count++;
    }

    // Re-arm when below low threshold
    if (m_state.order_param < m_triad_low) {
        // Ready for next crossing
    }

    // Check unlock condition
    if (m_state.triad_count >= TRIAD_PASSES_REQUIRED) {
        m_state.triad_unlocked = true;
    }
}

void KuramotoStabilizer::updateFromZ(float z) {
    // Set reference frequency based on z
    m_status.reference_freq = zToReferenceFreq(z);
}

float KuramotoStabilizer::zToReferenceFreq(float z) {
    // Map z ∈ [0, 1] to frequency range
    // Could use Solfeggio mapping or linear scaling

    // Linear mapping: 1 Hz at z=0, 10 Hz at z=1
    return 1.0f + z * 9.0f;
}

void KuramotoStabilizer::setCoupling(float K) {
    if (K < 0.0f) K = 0.0f;
    if (K > 1.0f) K = 1.0f;
    m_state.coupling = K;

    // Write to hardware
    writeCouplingHardware(K);
}

MagneticField KuramotoStabilizer::readMagneticField() {
    MagneticField field;
    memset(&field, 0, sizeof(field));

    if (!m_mag_initialized) return field;

    // Request data from HMC5883L
    Wire.beginTransmission(I2C::HMC5883L_ADDR);
    Wire.write(HMC5883L_DATA_X_H);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)I2C::HMC5883L_ADDR, (uint8_t)6);

    if (Wire.available() >= 6) {
        int16_t x = (Wire.read() << 8) | Wire.read();
        int16_t z = (Wire.read() << 8) | Wire.read();  // Note: HMC order is X, Z, Y
        int16_t y = (Wire.read() << 8) | Wire.read();

        // Convert to microtesla (approximate, depends on gain setting)
        const float scale = 0.092f;  // For gain = 1090 LSB/Gauss
        field.x = x * scale;
        field.y = y * scale;
        field.z = z * scale;

        field.magnitude = sqrtf(field.x * field.x + field.y * field.y + field.z * field.z);
        field.inclination = atan2f(field.z, sqrtf(field.x * field.x + field.y * field.y));
        field.declination = atan2f(field.y, field.x);
    }

    m_status.magnetic = field;
    return field;
}

float KuramotoStabilizer::applyMagneticModulation(float K_base) {
    MagneticField field = readMagneticField();

    // Coupling increases with field strength
    const float alpha = 0.0005f;  // Sensitivity coefficient
    float K_eff = K_base + alpha * field.magnitude;

    if (K_eff > 1.0f) K_eff = 1.0f;
    if (K_eff < 0.1f) K_eff = 0.1f;

    return K_eff;
}

void KuramotoStabilizer::setReferenceFrequency(float freq) {
    if (freq > 0.0f && freq < 1000.0f) {
        m_status.reference_freq = freq;
    }
}

void KuramotoStabilizer::reset() {
    // Re-initialize with default phases
    for (uint8_t i = 0; i < N_OSCILLATORS; i++) {
        m_state.phases[i] = static_cast<float>(i) * TWO_PI / N_OSCILLATORS;
    }

    m_state.triad_count = 0;
    m_state.triad_unlocked = false;
    m_state.order_param = 0.0f;
    m_state.collective_phase = 0.0f;

    m_pll_integrator = 0.0f;
    m_pll_proportional = 0.0f;

    m_status.is_synchronized = false;
    m_status.is_stabilized = false;
    m_status.pll_locked = false;
    m_status.sync_duration = 0;
}

bool KuramotoStabilizer::checkKFormation(float eta, uint8_t R) const {
    return (m_state.order_param >= K_KAPPA) &&
           (eta > K_ETA) &&
           (R >= K_R);
}

void KuramotoStabilizer::onSynchronization(SyncCallback callback) {
    m_sync_callback = callback;
}

void KuramotoStabilizer::setTriadThresholds(float high, float low) {
    if (high > low && high <= 1.0f && low >= 0.0f) {
        m_triad_high = high;
        m_triad_low = low;
    }
}

void KuramotoStabilizer::writeCouplingHardware(float K) {
    // Write to MCP41010 digipot
    // K=0 → R=10kΩ → step 255
    // K=1 → R=0 → step 0

    uint8_t step = static_cast<uint8_t>((1.0f - K) * 255);

    // SPI transaction to digipot
    digitalWrite(Pins::SPI_CS_DIGIPOT, LOW);

    // Command byte: write to potentiometer
    SPI.transfer(0x11);  // Write command
    SPI.transfer(step);

    digitalWrite(Pins::SPI_CS_DIGIPOT, HIGH);
}

} // namespace UCF

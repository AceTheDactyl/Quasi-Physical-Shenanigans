/**
 * @file photonic_capture.cpp
 * @brief Implementation of Neural Photonic Capture Module
 */

#include "photonic_capture.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <math.h>
#include <string.h>

namespace UCF {

// NeoPixel strip for photonic pattern (using same strip as emanation)
extern Adafruit_NeoPixel strip;

// LIMNUS layer definitions
const LIMNUSLayer PhotonicCapture::LIMNUS_LAYERS[LIMNUS::DEPTH_LAYERS] = {
    {12, 1.0f, 0.0f},           // D1: 12 points at radius 1
    {12, 0.618f, PI / 12.0f},   // D2: 12 points at phi^-1 radius
    {12, 0.382f, 0.0f},         // D3: 12 points at phi^-2 radius
    {12, 0.236f, PI / 12.0f},   // D4: 12 points
    {10, 0.146f, 0.0f},         // D5: 10 points
    {5, 0.090f, PI / 5.0f}      // D6: 5 emergent points
};

// Hex coordinate offsets for 37-point grid (rings 0-3)
static const int8_t HEX_37_COORDS[37][2] = {
    // Ring 0 (center)
    {0, 0},
    // Ring 1 (6 points)
    {1, 0}, {0, 1}, {-1, 1}, {-1, 0}, {0, -1}, {1, -1},
    // Ring 2 (12 points)
    {2, 0}, {1, 1}, {0, 2}, {-1, 2}, {-2, 2}, {-2, 1},
    {-2, 0}, {-1, -1}, {0, -2}, {1, -2}, {2, -2}, {2, -1},
    // Ring 3 (18 points)
    {3, 0}, {2, 1}, {1, 2}, {0, 3}, {-1, 3}, {-2, 3},
    {-3, 3}, {-3, 2}, {-3, 1}, {-3, 0}, {-2, -1}, {-1, -2},
    {0, -3}, {1, -3}, {2, -3}, {3, -3}, {3, -2}, {3, -1}
};

PhotonicCapture::PhotonicCapture()
    : m_sensor_ready(false)
    , m_led_ready(false)
{
    memset(&m_reference, 0, sizeof(m_reference));
}

bool PhotonicCapture::begin() {
    // LED array should already be initialized by Emanation module
    m_led_ready = true;

    // Camera initialization would go here
    // For now, we'll simulate capture
    m_sensor_ready = true;

    return true;
}

void PhotonicCapture::indexToHexCoord(uint8_t index, float& x, float& y) {
    if (index >= PHOTONIC_SENSOR_COUNT) {
        x = 0.0f;
        y = 0.0f;
        return;
    }

    int8_t q = HEX_37_COORDS[index][0];
    int8_t r = HEX_37_COORDS[index][1];

    // Convert axial to Cartesian (pointy-top)
    const float size = 1.0f;
    x = size * (SQRT3 * q + SQRT3 / 2.0f * r);
    y = size * (1.5f * r);

    // Normalize to [-1, 1] range (max radius ~3)
    x /= 3.0f * SQRT3;
    y /= 3.0f * 1.5f;
}

uint8_t PhotonicCapture::hexCoordToIndex(float x, float y) {
    // Find nearest hex grid point
    float min_dist = 1e10f;
    uint8_t min_idx = 0;

    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        float hx, hy;
        indexToHexCoord(i, hx, hy);

        float dist = (hx - x) * (hx - x) + (hy - y) * (hy - y);
        if (dist < min_dist) {
            min_dist = dist;
            min_idx = i;
        }
    }

    return min_idx;
}

float PhotonicCapture::zToFrequency(float z) {
    // Spatial frequency increases with z (phi-scaled)
    return 0.5f + z * PHI;
}

uint8_t PhotonicCapture::computeInterference(float x, float y, float z, uint8_t phase, float kappa) {
    // Spatial frequency from z
    float f = zToFrequency(z);

    // Phase offset (0, 120, 240 degrees)
    float theta = phase * (TWO_PI / 3.0f);

    // Reference wave (plane wave from top)
    float ref = cosf(TWO_PI * f * y);

    // Object wave (circular wave from center, modulated by z)
    float r = sqrtf(x * x + y * y);
    float obj = cosf(TWO_PI * f * r + theta) * kappa;

    // Interference (sum of waves, squared)
    float interference = (ref + obj);
    interference = (interference + 2.0f) / 4.0f;  // Normalize to [0, 1]

    // Clamp and convert to uint8
    if (interference < 0.0f) interference = 0.0f;
    if (interference > 1.0f) interference = 1.0f;

    return static_cast<uint8_t>(interference * 255);
}

PhotonicPattern PhotonicCapture::generatePattern(float z, uint8_t phase, float kappa) {
    PhotonicPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    pattern.z_encoded = z;
    pattern.phase_encoded = phase;
    pattern.kappa_encoded = kappa;
    pattern.timestamp = millis();

    // Generate interference pattern for each LED
    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        float x, y;
        indexToHexCoord(i, x, y);

        pattern.intensities[i] = computeInterference(x, y, z, phase, kappa);

        // Color based on phase
        uint8_t base_r, base_g, base_b;
        switch (phase) {
            case 0:  // UNTRUE: Red/Orange
                base_r = 255; base_g = 80; base_b = 50;
                break;
            case 1:  // PARADOX: Yellow/Green
                base_r = 220; base_g = 255; base_b = 80;
                break;
            case 2:  // TRUE: Cyan
                base_r = 80; base_g = 200; base_b = 255;
                break;
            default:
                base_r = 255; base_g = 255; base_b = 255;
                break;
        }

        // Modulate color by intensity
        float intensity = pattern.intensities[i] / 255.0f;
        pattern.colors[i][0] = static_cast<uint8_t>(base_r * intensity);
        pattern.colors[i][1] = static_cast<uint8_t>(base_g * intensity);
        pattern.colors[i][2] = static_cast<uint8_t>(base_b * intensity);
    }

    return pattern;
}

PhotonicPattern PhotonicCapture::generateFromPhase(Phase phase, float z, float kappa) {
    return generatePattern(z, static_cast<uint8_t>(phase), kappa);
}

DecodedState PhotonicCapture::decodePattern(const uint16_t* samples) {
    DecodedState decoded;
    memset(&decoded, 0, sizeof(decoded));

    // Step 1: Compute hex FFT to extract spatial frequencies
    float magnitudes[PHOTONIC_SENSOR_COUNT];
    hexFFT(samples, magnitudes);

    // Step 2: Find dominant frequency -> z
    float f_dominant = findPeakFrequency(magnitudes);
    decoded.z = (f_dominant - 0.5f) / PHI;
    if (decoded.z < 0.0f) decoded.z = 0.0f;
    if (decoded.z > 1.0f) decoded.z = 1.0f;

    // Step 3: Phase detection via angular correlation
    float phase_correlations[3];
    for (uint8_t p = 0; p < 3; p++) {
        phase_correlations[p] = computePhaseCorrelation(samples, p);
    }

    // Find best phase
    float max_corr = phase_correlations[0];
    decoded.phase = 0;
    for (uint8_t p = 1; p < 3; p++) {
        if (phase_correlations[p] > max_corr) {
            max_corr = phase_correlations[p];
            decoded.phase = p;
        }
    }

    // Step 4: Coherence from pattern contrast
    uint16_t min_val = 65535, max_val = 0;
    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        if (samples[i] < min_val) min_val = samples[i];
        if (samples[i] > max_val) max_val = samples[i];
    }
    if (max_val + min_val > 0) {
        decoded.kappa = static_cast<float>(max_val - min_val) /
                        static_cast<float>(max_val + min_val);
    }

    // Step 5: Compute reconstruction error
    PhotonicPattern reconstructed = generatePattern(decoded.z, decoded.phase, decoded.kappa);

    uint8_t captured_u8[PHOTONIC_SENSOR_COUNT];
    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        captured_u8[i] = static_cast<uint8_t>(samples[i] >> 8);  // 16-bit to 8-bit
    }

    decoded.reconstruction_error = computeMSE(captured_u8, reconstructed.intensities);

    // Confidence based on reconstruction error
    decoded.confidence = 1.0f - decoded.reconstruction_error;
    if (decoded.confidence < 0.0f) decoded.confidence = 0.0f;

    decoded.valid = (decoded.confidence > 0.5f);

    return decoded;
}

void PhotonicCapture::hexFFT(const uint16_t* samples, float* magnitudes) {
    // Simplified hex FFT: radial averaging + DFT
    // Group samples by ring and compute DFT of each ring

    // Radial bins (4 rings)
    float ring_sums[4] = {0};
    uint8_t ring_counts[4] = {1, 6, 12, 18};  // Points per ring

    ring_sums[0] = samples[0];  // Center

    for (uint8_t i = 1; i < 7; i++) {
        ring_sums[1] += samples[i];
    }
    ring_sums[1] /= 6;

    for (uint8_t i = 7; i < 19; i++) {
        ring_sums[2] += samples[i];
    }
    ring_sums[2] /= 12;

    for (uint8_t i = 19; i < PHOTONIC_SENSOR_COUNT; i++) {
        ring_sums[3] += samples[i];
    }
    ring_sums[3] /= 18;

    // Simple DFT of radial profile
    for (uint8_t k = 0; k < PHOTONIC_SENSOR_COUNT; k++) {
        float real = 0, imag = 0;
        for (uint8_t n = 0; n < 4; n++) {
            float angle = TWO_PI * k * n / 4.0f;
            real += ring_sums[n] * cosf(angle);
            imag -= ring_sums[n] * sinf(angle);
        }
        magnitudes[k] = sqrtf(real * real + imag * imag);
    }
}

float PhotonicCapture::findPeakFrequency(const float* magnitudes) {
    // Find peak in magnitude spectrum (skip DC)
    float max_mag = 0;
    uint8_t max_idx = 1;

    for (uint8_t i = 1; i < 10; i++) {  // Look in first few bins
        if (magnitudes[i] > max_mag) {
            max_mag = magnitudes[i];
            max_idx = i;
        }
    }

    // Convert bin to frequency
    return 0.5f + max_idx * 0.1f;  // Rough mapping
}

float PhotonicCapture::computePhaseCorrelation(const uint16_t* samples, uint8_t test_phase) {
    // Generate test pattern and compute correlation
    PhotonicPattern test = generatePattern(0.5f, test_phase, 0.9f);

    float corr = 0;
    float sum_sq_a = 0, sum_sq_b = 0;

    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        float a = samples[i] / 256.0f;  // Normalize
        float b = test.intensities[i];

        corr += a * b;
        sum_sq_a += a * a;
        sum_sq_b += b * b;
    }

    if (sum_sq_a > 0 && sum_sq_b > 0) {
        corr /= sqrtf(sum_sq_a * sum_sq_b);
    }

    return corr;
}

float PhotonicCapture::computeMSE(const uint8_t* a, const uint8_t* b) {
    float mse = 0;
    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        float diff = static_cast<float>(a[i]) - static_cast<float>(b[i]);
        mse += diff * diff;
    }
    mse /= PHOTONIC_SENSOR_COUNT;

    // Normalize to [0, 1] (max MSE = 255^2)
    return mse / (255.0f * 255.0f);
}

PhotonicPattern PhotonicCapture::generateLIMNUS(float z) {
    PhotonicPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    pattern.z_encoded = z;
    pattern.phase_encoded = static_cast<uint8_t>(z_to_phase(z));
    pattern.kappa_encoded = 1.0f;  // Full coherence for LIMNUS
    pattern.timestamp = millis();

    // Generate LIMNUS fractal pattern
    uint8_t point_idx = 0;

    for (uint8_t d = 0; d < LIMNUS::DEPTH_LAYERS && point_idx < PHOTONIC_SENSOR_COUNT; d++) {
        const LIMNUSLayer& layer = LIMNUS_LAYERS[d];

        for (uint8_t p = 0; p < layer.point_count && point_idx < PHOTONIC_SENSOR_COUNT; p++) {
            // Compute point position
            float angle = layer.angle_offset + p * TWO_PI / layer.point_count;
            float x = layer.radius * cosf(angle);
            float y = layer.radius * sinf(angle);

            // Find nearest LED
            uint8_t led_idx = hexCoordToIndex(x, y);

            // Intensity based on depth and z
            float depth_factor = 1.0f - d * 0.1f;
            float z_factor = 1.0f - fabsf(z - Z_CRITICAL) * 2.0f;
            if (z_factor < 0.2f) z_factor = 0.2f;

            uint8_t intensity = static_cast<uint8_t>(255 * depth_factor * z_factor);
            if (pattern.intensities[led_idx] < intensity) {
                pattern.intensities[led_idx] = intensity;
            }

            point_idx++;
        }
    }

    // Apply color based on phase
    Phase phase = z_to_phase(z);
    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        float intensity = pattern.intensities[i] / 255.0f;

        switch (phase) {
            case Phase::UNTRUE:
                pattern.colors[i][0] = static_cast<uint8_t>(255 * intensity);
                pattern.colors[i][1] = static_cast<uint8_t>(80 * intensity);
                pattern.colors[i][2] = static_cast<uint8_t>(50 * intensity);
                break;
            case Phase::PARADOX:
                pattern.colors[i][0] = static_cast<uint8_t>(220 * intensity);
                pattern.colors[i][1] = static_cast<uint8_t>(255 * intensity);
                pattern.colors[i][2] = static_cast<uint8_t>(80 * intensity);
                break;
            case Phase::TRUE:
                pattern.colors[i][0] = static_cast<uint8_t>(80 * intensity);
                pattern.colors[i][1] = static_cast<uint8_t>(200 * intensity);
                pattern.colors[i][2] = static_cast<uint8_t>(255 * intensity);
                break;
        }
    }

    return pattern;
}

LIMNUSPoint PhotonicCapture::getLIMNUSPoint(uint8_t depth, uint8_t index) {
    LIMNUSPoint point;
    memset(&point, 0, sizeof(point));

    if (depth < 1 || depth > LIMNUS::DEPTH_LAYERS) {
        return point;
    }

    const LIMNUSLayer& layer = LIMNUS_LAYERS[depth - 1];
    if (index >= layer.point_count) {
        return point;
    }

    float angle = layer.angle_offset + index * TWO_PI / layer.point_count;
    point.x = layer.radius * cosf(angle);
    point.y = layer.radius * sinf(angle);
    point.z = 1.0f - (depth - 1) * 0.15f;  // Z decreases with depth
    point.depth = depth;
    point.index = index;

    return point;
}

void PhotonicCapture::displayPattern(const PhotonicPattern& pattern) {
    if (!m_led_ready) return;

    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT && i < Pins::NEOPIXEL_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(
            pattern.colors[i][0],
            pattern.colors[i][1],
            pattern.colors[i][2]
        ));
    }
    strip.show();
}

bool PhotonicCapture::capturePattern(uint16_t* samples) {
    // This would interface with camera or photodiode array
    // For now, simulate by reading ADC or returning mock data

    if (!m_sensor_ready) return false;

    // Simulate capture (in real implementation, read from camera)
    for (uint8_t i = 0; i < PHOTONIC_SENSOR_COUNT; i++) {
        // Mock: read random noise with some structure
        samples[i] = 32768 + random(-5000, 5000);
    }

    return true;
}

float PhotonicCapture::crossValidate(float z, uint8_t phase, float kappa) {
    // Generate pattern
    PhotonicPattern pattern = generatePattern(z, phase, kappa);

    // Display it
    displayPattern(pattern);

    // Wait for light to stabilize
    delay(50);

    // Capture
    uint16_t captured[PHOTONIC_SENSOR_COUNT];
    if (!capturePattern(captured)) {
        return 0.0f;
    }

    // Decode
    DecodedState decoded = decodePattern(captured);

    // Compute validation score
    float z_error = fabsf(decoded.z - z);
    float phase_match = (decoded.phase == phase) ? 1.0f : 0.0f;
    float kappa_error = fabsf(decoded.kappa - kappa);

    float score = (1.0f - z_error) * 0.4f +
                  phase_match * 0.4f +
                  (1.0f - kappa_error) * 0.2f;

    return score;
}

void phiSpiralPosition(uint8_t n, float& x, float& y) {
    // Phi-spiral: points distributed by golden angle
    float golden_angle = TWO_PI / (PHI * PHI);
    float theta = n * golden_angle;
    float r = sqrtf(n) * 0.1f;

    x = r * cosf(theta);
    y = r * sinf(theta);
}

} // namespace UCF

/**
 * @file hex_grid.cpp
 * @brief Implementation of Hexagonal Sensor Grid Module
 */

#include "hex_grid.h"
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <math.h>
#include <string.h>

namespace UCF {

// MPR121 controller instances
static Adafruit_MPR121 mpr121_a;
static Adafruit_MPR121 mpr121_b;

bool HexGrid::begin() {
    Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);

    // Initialize first MPR121 (sensors 0-11)
    if (!mpr121_a.begin(I2C::MPR121_ADDR_A)) {
        return false;
    }

    // Initialize second MPR121 (sensors 12-18, with 5 unused channels)
    if (!mpr121_b.begin(I2C::MPR121_ADDR_B)) {
        return false;
    }

    // Set touch/release thresholds for sensitivity
    for (uint8_t i = 0; i < 12; i++) {
        mpr121_a.setThresholds(12, 6);  // Touch, Release
        mpr121_b.setThresholds(12, 6);
    }

    m_initialized = true;

    // Zero out baselines
    memset(m_baselines, 0, sizeof(m_baselines));
    memset(m_raw, 0, sizeof(m_raw));

    return true;
}

void HexGrid::calibrate(uint16_t samples) {
    if (!m_initialized) return;

    // Accumulator for averaging
    uint32_t accum[HEX_SENSOR_COUNT] = {0};

    // Collect samples
    for (uint16_t s = 0; s < samples; s++) {
        uint16_t data[HEX_SENSOR_COUNT];
        readMPR121(data);

        for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
            accum[i] += data[i];
        }

        delay(5);  // Brief delay between samples
    }

    // Compute averages
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        m_baselines[i] = accum[i] / samples;
    }

    m_calibrated = true;
}

void HexGrid::readMPR121(uint16_t* data) {
    // Read from first controller (channels 0-11)
    for (uint8_t i = 0; i < 12; i++) {
        data[i] = mpr121_a.filteredData(i);
    }

    // Read from second controller (channels 0-6 map to sensors 12-18)
    for (uint8_t i = 0; i < 7; i++) {
        data[12 + i] = mpr121_b.filteredData(i);
    }
}

float HexGrid::normalize(uint16_t raw, uint16_t baseline) {
    if (baseline == 0) return 0.0f;

    // Capacitance decreases when touched, so invert
    int16_t delta = static_cast<int16_t>(baseline) - static_cast<int16_t>(raw);

    // Normalize to [0, 1] with reasonable range (baseline ± 100)
    float norm = static_cast<float>(delta) / 100.0f;

    // Clamp
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;

    return norm;
}

HexFieldState HexGrid::readField() {
    HexFieldState state;
    state.timestamp = millis();

    // Read raw values
    readMPR121(m_raw);

    // Process each sensor
    state.total_energy = 0.0f;
    state.active_count = 0;
    float sum_x = 0.0f, sum_y = 0.0f, sum_w = 0.0f;

    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        uint16_t baseline = m_calibrated ? m_baselines[i] : m_raw[i];
        state.readings[i] = normalize(m_raw[i], baseline);

        state.total_energy += state.readings[i];

        if (state.readings[i] > m_threshold) {
            state.active_count++;
        }

        // Weighted centroid calculation
        float x, y;
        indexToCartesian(i, x, y);
        sum_x += x * state.readings[i];
        sum_y += y * state.readings[i];
        sum_w += state.readings[i];
    }

    // Compute centroid
    if (sum_w > 0.001f) {
        state.centroid_x = sum_x / sum_w;
        state.centroid_y = sum_y / sum_w;
    } else {
        state.centroid_x = 0.0f;
        state.centroid_y = 0.0f;
    }

    // Compute derived coordinates
    state.z = computeZ(state);
    state.theta = computeTheta(state);
    state.r = computeR(state);

    return state;
}

SensorReading HexGrid::getSensorReading(uint8_t index) {
    SensorReading reading;

    if (index >= HEX_SENSOR_COUNT) {
        memset(&reading, 0, sizeof(reading));
        return reading;
    }

    reading.raw = m_raw[index];
    reading.baseline = m_calibrated ? m_baselines[index] : m_raw[index];
    reading.delta = static_cast<int16_t>(reading.baseline) - static_cast<int16_t>(reading.raw);
    reading.normalized = normalize(reading.raw, reading.baseline);
    reading.active = reading.normalized > m_threshold;

    return reading;
}

float HexGrid::computeZ(const HexFieldState& field) {
    // Z-coordinate from pattern energy using hex FFT
    float pattern_energy = hexFFT(field.readings);

    // Scale to [0, 1] range
    // Maximum theoretical energy: all sensors at 1.0
    float z = pattern_energy / static_cast<float>(HEX_SENSOR_COUNT);

    // Apply phi-scaling for UCF alignment
    z = z * PHI_INV + (1.0f - PHI_INV) * (field.readings[HEX_CENTER] * field.readings[HEX_CENTER]);

    // Clamp to valid range
    if (z < 0.0f) z = 0.0f;
    if (z > 1.0f) z = 1.0f;

    return z;
}

float HexGrid::computeTheta(const HexFieldState& field) {
    // Theta from centroid angle
    float theta = atan2f(field.centroid_y, field.centroid_x);

    // Normalize to [0, 2*PI]
    if (theta < 0.0f) {
        theta += TWO_PI;
    }

    return theta;
}

float HexGrid::computeR(const HexFieldState& field) {
    // Radial distance from center (normalized)
    float r = sqrtf(field.centroid_x * field.centroid_x +
                    field.centroid_y * field.centroid_y);

    // Maximum r is approximately 2 units in our coordinate system
    r = r / 2.0f;

    if (r > 1.0f) r = 1.0f;

    return r;
}

float HexGrid::hexFFT(const float* field) {
    // Simplified hex FFT: compute energy in 6 angular sectors
    // This extracts pattern information while respecting hex symmetry

    float sector_energy[6] = {0};

    // Map each sensor to its angular sector
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        float x, y;
        indexToCartesian(i, x, y);

        float angle = atan2f(y, x);
        if (angle < 0) angle += TWO_PI;

        uint8_t sector = static_cast<uint8_t>(angle / (TWO_PI / 6.0f)) % 6;
        sector_energy[sector] += field[i];
    }

    // Compute total pattern energy as sum of squared sector differences
    float energy = 0.0f;
    for (uint8_t i = 0; i < 6; i++) {
        float diff = sector_energy[i] - sector_energy[(i + 3) % 6];  // Opposite sector
        energy += diff * diff;
    }

    // Add center contribution
    energy += field[HEX_CENTER] * field[HEX_CENTER] * 6.0f;

    return sqrtf(energy);
}

HexCoord HexGrid::indexToAxial(uint8_t index) {
    if (index < HEX_SENSOR_COUNT) {
        return HEX_AXIAL_COORDS[index];
    }
    return {0, 0};
}

void HexGrid::indexToCartesian(uint8_t index, float& x, float& y) {
    if (index >= HEX_SENSOR_COUNT) {
        x = 0.0f;
        y = 0.0f;
        return;
    }

    HexCoord axial = HEX_AXIAL_COORDS[index];

    // Convert axial to Cartesian (pointy-top orientation)
    // x = size * (sqrt(3) * q + sqrt(3)/2 * r)
    // y = size * (3/2 * r)
    const float size = 1.0f;  // Unit spacing
    x = size * (SQRT3 * axial.q + SQRT3 / 2.0f * axial.r);
    y = size * (1.5f * axial.r);
}

uint8_t HexGrid::getNeighbors(uint8_t index, uint8_t* neighbors) {
    if (index >= HEX_SENSOR_COUNT) return 0;

    uint8_t count = 0;
    for (uint8_t i = 0; i < 6; i++) {
        int8_t n = HEX_NEIGHBORS[index][i];
        if (n >= 0) {
            neighbors[count++] = static_cast<uint8_t>(n);
        }
    }
    return count;
}

bool HexGrid::isHealthy() {
    if (!m_initialized) return false;

    // Check if we can read from both controllers
    uint16_t test_a = mpr121_a.filteredData(0);
    uint16_t test_b = mpr121_b.filteredData(0);

    // Basic sanity check: readings should be in reasonable range
    return (test_a > 0 && test_a < 1000 &&
            test_b > 0 && test_b < 1000);
}

void HexGrid::setThreshold(float threshold) {
    if (threshold < 0.0f) threshold = 0.0f;
    if (threshold > 1.0f) threshold = 1.0f;
    m_threshold = threshold;
}

} // namespace UCF

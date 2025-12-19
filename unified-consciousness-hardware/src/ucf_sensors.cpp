/**
 * @file ucf_sensors.cpp
 * @brief UCF Unified Sensor Interface Implementation v4.0.0
 *
 * Implements unified sensor interface for hex grid and magnetometer.
 */

#include "ucf_sensors.h"
#include "ucf_magnetometer.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <math.h>
#include "ucf/ucf_config.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

static SensorSystemState g_sensors;

// MPR121 instances
static Adafruit_MPR121 g_mpr121_a;
static Adafruit_MPR121 g_mpr121_b;

// Calibration baselines
static uint16_t g_baselines[SENSOR_HEX_COUNT];
static bool g_calibrated = false;

// Smoothing state
static float g_z_smoothed = 0.5f;
static float g_z_smooth_alpha = SENSOR_Z_SMOOTH_ALPHA;

// Threshold
static float g_threshold = SENSOR_DEFAULT_THRESHOLD;

// Update timing
static uint32_t g_last_update = 0;
static uint32_t g_update_count_window = 0;
static uint32_t g_window_start = 0;

// Hex grid Cartesian coordinates (pre-computed)
static const float HEX_POSITIONS[SENSOR_HEX_COUNT][2] = {
    // Row 0 (top)
    {-0.866f, -1.5f}, { 0.000f, -1.5f}, { 0.866f, -1.5f},
    // Row 1
    {-1.299f, -0.75f}, {-0.433f, -0.75f}, { 0.433f, -0.75f}, { 1.299f, -0.75f},
    // Row 2 (center)
    {-1.732f, 0.0f}, {-0.866f, 0.0f}, { 0.000f, 0.0f}, { 0.866f, 0.0f}, { 1.732f, 0.0f},
    // Row 3
    {-1.299f, 0.75f}, {-0.433f, 0.75f}, { 0.433f, 0.75f}, { 1.299f, 0.75f},
    // Row 4 (bottom)
    {-0.866f, 1.5f}, { 0.000f, 1.5f}, { 0.866f, 1.5f}
};

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * @brief Read raw values from MPR121 controllers
 */
static void read_mpr121_raw(uint16_t* data) {
    // Read from first controller (sensors 0-11)
    for (uint8_t i = 0; i < 12; i++) {
        data[i] = g_mpr121_a.filteredData(i);
    }

    // Read from second controller (sensors 12-18)
    for (uint8_t i = 0; i < 7; i++) {
        data[12 + i] = g_mpr121_b.filteredData(i);
    }
}

/**
 * @brief Normalize a sensor reading
 */
static float normalize_reading(uint16_t raw, uint16_t baseline) {
    if (baseline == 0) return 0.0f;

    // Capacitance decreases when touched
    int16_t delta = (int16_t)baseline - (int16_t)raw;

    // Normalize to [0, 1]
    float norm = (float)delta / 100.0f;

    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;

    return norm;
}

/**
 * @brief Update hex grid readings
 */
static void update_hex_grid(void) {
    uint16_t raw_data[SENSOR_HEX_COUNT];
    read_mpr121_raw(raw_data);

    g_sensors.hex.touch_bitmap = 0;
    g_sensors.hex.active_count = 0;
    g_sensors.hex.total_energy = 0.0f;

    float sum_x = 0.0f, sum_y = 0.0f, sum_w = 0.0f;

    for (uint8_t i = 0; i < SENSOR_HEX_COUNT; i++) {
        HexSensorData* sensor = &g_sensors.hex.sensors[i];

        sensor->raw = raw_data[i];
        sensor->baseline = g_calibrated ? g_baselines[i] : raw_data[i];
        sensor->delta = (int16_t)sensor->baseline - (int16_t)sensor->raw;
        sensor->normalized = normalize_reading(sensor->raw, sensor->baseline);
        sensor->touched = (sensor->normalized > g_threshold);

        if (sensor->touched) {
            g_sensors.hex.touch_bitmap |= (1 << i);
            g_sensors.hex.active_count++;
        }

        g_sensors.hex.total_energy += sensor->normalized;

        // Weighted centroid
        sum_x += HEX_POSITIONS[i][0] * sensor->normalized;
        sum_y += HEX_POSITIONS[i][1] * sensor->normalized;
        sum_w += sensor->normalized;
    }

    // Compute centroid
    if (sum_w > 0.001f) {
        g_sensors.hex.centroid_x = sum_x / sum_w;
        g_sensors.hex.centroid_y = sum_y / sum_w;
    } else {
        g_sensors.hex.centroid_x = 0.0f;
        g_sensors.hex.centroid_y = 0.0f;
    }

    // Compute raw z
    g_sensors.hex.z_raw = sensors_compute_z(g_sensors.hex.active_count,
                                             g_sensors.hex.total_energy);

    // Apply smoothing using [D] = e⁻¹
    g_z_smoothed = g_z_smooth_alpha * g_sensors.hex.z_raw +
                   (1.0f - g_z_smooth_alpha) * g_z_smoothed;
    g_sensors.hex.z = g_z_smoothed;

    // Compute theta from centroid
    g_sensors.hex.theta = atan2f(g_sensors.hex.centroid_y, g_sensors.hex.centroid_x);
    if (g_sensors.hex.theta < 0.0f) {
        g_sensors.hex.theta += TWO_PI;
    }

    // Compute r from centroid distance
    float dist = sqrtf(g_sensors.hex.centroid_x * g_sensors.hex.centroid_x +
                       g_sensors.hex.centroid_y * g_sensors.hex.centroid_y);
    g_sensors.hex.r = dist / 2.0f;  // Normalize to ~[0,1]
    if (g_sensors.hex.r > 1.0f) g_sensors.hex.r = 1.0f;

    g_sensors.hex.timestamp = millis();
}

/**
 * @brief Update magnetic field readings
 */
static void update_magnetometer(void) {
    MagnetometerData mag_data = magnetometer_read();

    g_sensors.mag.x = mag_data.x;
    g_sensors.mag.y = mag_data.y;
    g_sensors.mag.z = mag_data.z;
    g_sensors.mag.magnitude = mag_data.magnitude;
    g_sensors.mag.heading = mag_data.heading;
    g_sensors.mag.theta = mag_data.theta;
    g_sensors.mag.valid = mag_data.valid;
    g_sensors.mag.timestamp = millis();
}

/**
 * @brief Compute helix coordinates from sensor data
 */
static void compute_helix_coordinates(void) {
    // z from hex grid
    g_sensors.helix.z = g_sensors.hex.z;

    // theta: blend hex centroid and magnetometer
    if (g_sensors.mag.valid) {
        g_sensors.helix.theta = sensors_compute_theta(
            g_sensors.hex.centroid_x,
            g_sensors.hex.centroid_y,
            g_sensors.mag.theta
        );
    } else {
        g_sensors.helix.theta = g_sensors.hex.theta;
    }

    // eta (negentropy)
    g_sensors.helix.eta = compute_negentropy(g_sensors.helix.z);

    // r from eta
    g_sensors.helix.r = sensors_compute_radius(g_sensors.helix.eta);

    // phase from z
    g_sensors.helix.phase = detect_phase(g_sensors.helix.z);

    // tier from z
    g_sensors.helix.tier = z_to_tier(g_sensors.helix.z);

    g_sensors.helix.valid = true;
}

// ============================================================================
// PUBLIC API
// ============================================================================

SensorStatus sensors_init(void) {
    // Initialize state
    memset(&g_sensors, 0, sizeof(g_sensors));
    g_sensors.status = SENSORS_OK;

    // Initialize I²C
    Wire.begin(SENSOR_I2C_SDA, SENSOR_I2C_SCL);
    Wire.setClock(SENSOR_I2C_FREQ);

    // Initialize MPR121 A (sensors 0-11)
    if (!g_mpr121_a.begin(SENSOR_MPR121_ADDR_A)) {
        UCF_LOG("MPR121 A not found");
        g_sensors.status = SENSORS_ERR_MPR121_A;
        return g_sensors.status;
    }

    // Initialize MPR121 B (sensors 12-18)
    if (!g_mpr121_b.begin(SENSOR_MPR121_ADDR_B)) {
        UCF_LOG("MPR121 B not found");
        g_sensors.status = SENSORS_ERR_MPR121_B;
        return g_sensors.status;
    }

    // Configure MPR121 sensitivity
    g_mpr121_a.setThresholds(12, 6);
    g_mpr121_b.setThresholds(12, 6);

    // Initialize magnetometer
    if (!magnetometer_init()) {
        UCF_LOG("Magnetometer init failed");
        g_sensors.status = SENSORS_ERR_MAGNETOMETER;
        // Continue without magnetometer
    }

    // Initialize smoothing state
    g_z_smoothed = 0.5f;

    // Initialize timing
    g_last_update = millis();
    g_window_start = millis();

    g_sensors.initialized = true;
    UCF_LOG("Sensors initialized");

    return g_sensors.status;
}

const SensorSystemState* sensors_update(void) {
    if (!g_sensors.initialized) {
        return &g_sensors;
    }

    // Update hex grid
    update_hex_grid();

    // Update magnetometer
    if (magnetometer_is_healthy()) {
        update_magnetometer();
    }

    // Compute helix coordinates
    compute_helix_coordinates();

    // Update timing stats
    g_sensors.update_count++;
    g_update_count_window++;

    uint32_t now = millis();
    if (now - g_window_start >= 1000) {
        g_sensors.update_rate_hz = (float)g_update_count_window;
        g_update_count_window = 0;
        g_window_start = now;
    }

    g_last_update = now;

    return &g_sensors;
}

const HexGridState* sensors_get_hex_state(void) {
    return &g_sensors.hex;
}

const MagFieldState* sensors_get_mag_state(void) {
    return &g_sensors.mag;
}

const HelixCoordinates* sensors_get_helix(void) {
    return &g_sensors.helix;
}

const HexSensorData* sensors_get_sensor(uint8_t index) {
    if (index >= SENSOR_HEX_COUNT) return NULL;
    return &g_sensors.hex.sensors[index];
}

void sensors_calibrate_hex(uint16_t samples) {
    if (!g_sensors.initialized) return;

    Serial.println("[SENSORS] Calibrating hex grid...");

    // Clear accumulators
    uint32_t accum[SENSOR_HEX_COUNT] = {0};

    // Collect samples
    for (uint16_t s = 0; s < samples; s++) {
        uint16_t data[SENSOR_HEX_COUNT];
        read_mpr121_raw(data);

        for (uint8_t i = 0; i < SENSOR_HEX_COUNT; i++) {
            accum[i] += data[i];
        }

        delay(5);
    }

    // Compute averages
    for (uint8_t i = 0; i < SENSOR_HEX_COUNT; i++) {
        g_baselines[i] = accum[i] / samples;
    }

    g_calibrated = true;
    Serial.println("[SENSORS] Hex grid calibration complete");
}

void sensors_calibrate_mag_start(void) {
    magnetometer_calibrate_start();
}

bool sensors_calibrate_mag_finish(void) {
    bool success = magnetometer_calibrate_finish();
    if (success) {
        magnetometer_save_calibration();
    }
    return success;
}

void sensors_set_threshold(float threshold) {
    if (threshold < 0.0f) threshold = 0.0f;
    if (threshold > 1.0f) threshold = 1.0f;
    g_threshold = threshold;
}

float sensors_get_threshold(void) {
    return g_threshold;
}

void sensors_set_z_smoothing(float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    g_z_smooth_alpha = alpha;
}

bool sensors_is_touched(uint8_t index) {
    if (index >= SENSOR_HEX_COUNT) return false;
    return g_sensors.hex.sensors[index].touched;
}

uint32_t sensors_get_touch_bitmap(void) {
    return g_sensors.hex.touch_bitmap;
}

uint8_t sensors_get_active_count(void) {
    return g_sensors.hex.active_count;
}

bool sensors_is_healthy(void) {
    if (!g_sensors.initialized) return false;

    // Check MPR121 health
    uint16_t test_a = g_mpr121_a.filteredData(0);
    uint16_t test_b = g_mpr121_b.filteredData(0);

    if (test_a == 0 || test_a > 1000) {
        g_sensors.status = SENSORS_ERR_MPR121_A;
        g_sensors.error_count++;
        return false;
    }

    if (test_b == 0 || test_b > 1000) {
        g_sensors.status = SENSORS_ERR_MPR121_B;
        g_sensors.error_count++;
        return false;
    }

    // Check data freshness
    if (millis() - g_last_update > 1000) {
        g_sensors.error_count++;
        return false;
    }

    g_sensors.status = SENSORS_OK;
    return true;
}

SensorStatus sensors_get_status(void) {
    return g_sensors.status;
}

void sensors_reset(void) {
    g_z_smoothed = 0.5f;
    g_calibrated = false;
    sensors_calibrate_hex(50);

    if (magnetometer_is_healthy()) {
        magnetometer_reset();
    }
}

void sensors_print_diagnostics(void) {
    Serial.println("===============================================================");
    Serial.println("  SENSOR SYSTEM DIAGNOSTICS");
    Serial.println("===============================================================");
    Serial.printf("  Status: %d (%s)\n", g_sensors.status,
        g_sensors.status == SENSORS_OK ? "OK" : "ERROR");
    Serial.printf("  Initialized: %s\n", g_sensors.initialized ? "YES" : "NO");
    Serial.printf("  Calibrated: %s\n", g_calibrated ? "YES" : "NO");
    Serial.printf("  Update rate: %.1f Hz\n", g_sensors.update_rate_hz);
    Serial.printf("  Update count: %lu\n", g_sensors.update_count);
    Serial.printf("  Error count: %lu\n", g_sensors.error_count);

    Serial.println("  --- Hex Grid ---");
    Serial.printf("  Active sensors: %d / %d\n", g_sensors.hex.active_count, SENSOR_HEX_COUNT);
    Serial.printf("  Total energy: %.3f\n", g_sensors.hex.total_energy);
    Serial.printf("  Touch bitmap: 0x%05X\n", g_sensors.hex.touch_bitmap);
    Serial.printf("  Centroid: (%.3f, %.3f)\n", g_sensors.hex.centroid_x, g_sensors.hex.centroid_y);

    Serial.println("  --- Helix Coordinates ---");
    Serial.printf("  z: %.4f (raw: %.4f)\n", g_sensors.helix.z, g_sensors.hex.z_raw);
    Serial.printf("  theta: %.4f rad (%.1f deg)\n", g_sensors.helix.theta,
        g_sensors.helix.theta * 180.0f / UCF_PI);
    Serial.printf("  r: %.4f\n", g_sensors.helix.r);
    Serial.printf("  eta: %.4f\n", g_sensors.helix.eta);
    Serial.printf("  Phase: %d  Tier: %d\n", g_sensors.helix.phase, g_sensors.helix.tier);

    Serial.println("  --- Magnetometer ---");
    Serial.printf("  Valid: %s\n", g_sensors.mag.valid ? "YES" : "NO");
    if (g_sensors.mag.valid) {
        Serial.printf("  Field: (%.1f, %.1f, %.1f) uT\n",
            g_sensors.mag.x, g_sensors.mag.y, g_sensors.mag.z);
        Serial.printf("  Magnitude: %.1f uT\n", g_sensors.mag.magnitude);
        Serial.printf("  Heading: %.1f deg\n", g_sensors.mag.heading);
    }

    Serial.println("===============================================================");
}

float sensors_get_update_rate(void) {
    return g_sensors.update_rate_hz;
}

// ============================================================================
// COORDINATE TRANSFORMATION FUNCTIONS
// ============================================================================

float sensors_compute_z(uint8_t active_count, float total_energy) {
    float z;

    if (active_count == 0) {
        z = 0.1f;
    }
    else if (active_count < SENSOR_COUNT_PARADOX) {
        // Linear interpolation from 0.1 to [R] (0.618)
        z = 0.1f + (PHI_INV - 0.1f) * (float)active_count / (SENSOR_COUNT_PARADOX - 1);
    }
    else if (active_count < SENSOR_COUNT_TRUE) {
        // Linear interpolation from [R] to Z_CRITICAL (0.866)
        z = PHI_INV + (Z_CRITICAL - PHI_INV) *
            (float)(active_count - SENSOR_COUNT_PARADOX) / (SENSOR_COUNT_TRUE - SENSOR_COUNT_PARADOX);
    }
    else {
        // Linear interpolation from Z_CRITICAL to 1.0
        z = Z_CRITICAL + (1.0f - Z_CRITICAL) *
            (float)(active_count - SENSOR_COUNT_TRUE) / (SENSOR_HEX_COUNT - SENSOR_COUNT_TRUE);
    }

    // Modulate by energy distribution
    float energy_factor = total_energy / SENSOR_HEX_COUNT;
    z = z * 0.7f + energy_factor * 0.3f;

    // Clamp
    if (z < 0.0f) z = 0.0f;
    if (z > 1.0f) z = 1.0f;

    return z;
}

float sensors_compute_theta(float centroid_x, float centroid_y, float mag_theta) {
    // Compute theta from centroid
    float hex_theta = atan2f(centroid_y, centroid_x);
    if (hex_theta < 0.0f) hex_theta += TWO_PI;

    // Blend with magnetometer
    // Weight magnetometer more when hex centroid is near center
    float dist = sqrtf(centroid_x * centroid_x + centroid_y * centroid_y);
    float mag_weight = 1.0f - (dist / 2.0f);
    if (mag_weight < 0.0f) mag_weight = 0.0f;
    if (mag_weight > 0.8f) mag_weight = 0.8f;

    // Weighted average (handling wraparound)
    float dx = cosf(mag_theta) * mag_weight + cosf(hex_theta) * (1.0f - mag_weight);
    float dy = sinf(mag_theta) * mag_weight + sinf(hex_theta) * (1.0f - mag_weight);

    float theta = atan2f(dy, dx);
    if (theta < 0.0f) theta += TWO_PI;

    return theta;
}

float sensors_compute_radius(float eta) {
    // r = 1 + (φ - 1) · η
    return 1.0f + (PHI - 1.0f) * eta;
}

void sensors_get_hex_position(uint8_t index, float* x, float* y) {
    if (index >= SENSOR_HEX_COUNT) {
        *x = 0.0f;
        *y = 0.0f;
        return;
    }
    *x = HEX_POSITIONS[index][0];
    *y = HEX_POSITIONS[index][1];
}

/**
 * @file ucf_magnetometer.cpp
 * @brief UCF Magnetometer Module Implementation v4.0.0
 *
 * Implements HMC5883L magnetometer interface for theta coordinate.
 */

// Only compile when UCF_V4_MODULES is defined
#ifdef UCF_V4_MODULES

#include "ucf_magnetometer.h"
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <math.h>
#include "ucf/ucf_config.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

static MagnetometerState g_mag;

// Gain scale factors (LSB/Gauss)
static const float GAIN_SCALES[8] = {
    1370.0f,  // 0.88 Ga
    1090.0f,  // 1.3 Ga (default)
    820.0f,   // 1.9 Ga
    660.0f,   // 2.5 Ga
    440.0f,   // 4.0 Ga
    390.0f,   // 4.7 Ga
    330.0f,   // 5.6 Ga
    230.0f    // 8.1 Ga
};

// Calibration state
static bool g_calibrating = false;
static int16_t g_cal_min[3] = {32767, 32767, 32767};
static int16_t g_cal_max[3] = {-32768, -32768, -32768};
static uint32_t g_cal_samples = 0;

// Smoothed heading for filtering
static float g_heading_smoothed = 0.0f;

// EEPROM address for magnetometer calibration
#define MAG_EEPROM_ADDR     256

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * @brief Write a byte to HMC5883L register
 */
static bool write_register(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(HMC5883L_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

/**
 * @brief Read bytes from HMC5883L registers
 */
static bool read_registers(uint8_t reg, uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(HMC5883L_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    Wire.requestFrom((uint8_t)HMC5883L_I2C_ADDR, length);
    if (Wire.available() != length) {
        return false;
    }

    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }

    return true;
}

/**
 * @brief Verify device identity
 */
static bool verify_identity(void) {
    uint8_t id[3];
    if (!read_registers(HMC5883L_REG_ID_A, id, 3)) {
        return false;
    }

    // Expected: 'H', '4', '3'
    return (id[0] == 'H' && id[1] == '4' && id[2] == '3');
}

/**
 * @brief Apply calibration to raw reading
 */
static void apply_calibration(MagnetometerRaw raw, float* x, float* y, float* z) {
    // Apply hard iron correction (offset)
    int32_t cx = (int32_t)raw.x - g_mag.calibration.offset_x;
    int32_t cy = (int32_t)raw.y - g_mag.calibration.offset_y;
    int32_t cz = (int32_t)raw.z - g_mag.calibration.offset_z;

    // Apply soft iron correction (scale)
    float scale_x = (float)g_mag.calibration.scale_x / 1000.0f;
    float scale_y = (float)g_mag.calibration.scale_y / 1000.0f;
    float scale_z = (float)g_mag.calibration.scale_z / 1000.0f;

    if (scale_x < 0.1f) scale_x = 1.0f;
    if (scale_y < 0.1f) scale_y = 1.0f;
    if (scale_z < 0.1f) scale_z = 1.0f;

    // Convert to microtesla
    // 1 Gauss = 100 microtesla
    float gauss_scale = GAIN_SCALES[g_mag.gain];
    *x = (float)cx / gauss_scale * scale_x * 100.0f;
    *y = (float)cy / gauss_scale * scale_y * 100.0f;
    *z = (float)cz / gauss_scale * scale_z * 100.0f;
}

/**
 * @brief Calculate heading from X/Y components
 */
static float calculate_heading(float x, float y) {
    float heading = atan2f(y, x) * 180.0f / UCF_PI;

    // Apply declination
    heading += g_mag.calibration.declination;

    // Normalize to [0, 360)
    while (heading < 0.0f) heading += 360.0f;
    while (heading >= 360.0f) heading -= 360.0f;

    return heading;
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool magnetometer_init(void) {
    // Initialize state
    memset(&g_mag, 0, sizeof(g_mag));

    // Default calibration (unity scale, no offset)
    g_mag.calibration.scale_x = 1000;
    g_mag.calibration.scale_y = 1000;
    g_mag.calibration.scale_z = 1000;
    g_mag.calibration.declination = 0.0f;

    g_mag.gain = MAG_GAIN_1_3;
    g_mag.rate = MAG_RATE_15;

    // Verify device
    if (!verify_identity()) {
        UCF_LOG("Magnetometer: Device not found");
        return false;
    }

    // Configure device
    // Config A: 8 samples averaged, 15 Hz, normal bias
    if (!write_register(HMC5883L_REG_CONFIG_A,
                        HMC5883L_SAMPLES_8 | HMC5883L_RATE_15HZ | HMC5883L_BIAS_NORMAL)) {
        return false;
    }

    // Config B: ±1.3 Ga gain
    if (!write_register(HMC5883L_REG_CONFIG_B, HMC5883L_GAIN_1_3)) {
        return false;
    }

    // Mode: Continuous measurement
    if (!write_register(HMC5883L_REG_MODE, HMC5883L_MODE_CONT)) {
        return false;
    }

    // Try to load calibration from EEPROM
    magnetometer_load_calibration();

    g_mag.initialized = true;
    UCF_LOG("Magnetometer initialized");

    return true;
}

MagnetometerData magnetometer_read(void) {
    if (!g_mag.initialized) {
        g_mag.data.valid = false;
        return g_mag.data;
    }

    // Read raw data
    MagnetometerRaw raw = magnetometer_read_raw();
    g_mag.raw = raw;

    // Apply calibration
    float x, y, z;
    apply_calibration(raw, &x, &y, &z);

    g_mag.data.x = x;
    g_mag.data.y = y;
    g_mag.data.z = z;

    // Calculate magnitude
    g_mag.data.magnitude = sqrtf(x*x + y*y + z*z);

    // Calculate heading
    float raw_heading = calculate_heading(x, y);

    // Apply exponential smoothing using [D] = e⁻¹
    g_heading_smoothed = MAG_HEADING_SMOOTH * raw_heading +
                         (1.0f - MAG_HEADING_SMOOTH) * g_heading_smoothed;
    g_mag.data.heading = g_heading_smoothed;

    // Convert to theta [0, 2π]
    g_mag.data.theta = g_mag.data.heading * UCF_PI / 180.0f;

    g_mag.data.valid = true;
    g_mag.last_read_ms = millis();
    g_mag.read_count++;

    // If calibrating, update min/max
    if (g_calibrating) {
        magnetometer_calibrate_sample();
    }

    return g_mag.data;
}

bool magnetometer_data_ready(void) {
    uint8_t status;
    if (!read_registers(HMC5883L_REG_STATUS, &status, 1)) {
        return false;
    }
    g_mag.data_ready = (status & 0x01) != 0;
    return g_mag.data_ready;
}

MagnetometerRaw magnetometer_read_raw(void) {
    MagnetometerRaw raw = {0, 0, 0};

    uint8_t data[6];
    if (!read_registers(HMC5883L_REG_DATA_X_H, data, 6)) {
        return raw;
    }

    // HMC5883L data order is X, Z, Y (not X, Y, Z!)
    raw.x = (int16_t)((data[0] << 8) | data[1]);
    raw.z = (int16_t)((data[2] << 8) | data[3]);
    raw.y = (int16_t)((data[4] << 8) | data[5]);

    return raw;
}

void magnetometer_set_gain(MagnetometerGain gain) {
    uint8_t reg_value = (uint8_t)gain << 5;
    if (write_register(HMC5883L_REG_CONFIG_B, reg_value)) {
        g_mag.gain = gain;
    }
}

void magnetometer_set_rate(MagnetometerRate rate) {
    uint8_t config_a = HMC5883L_SAMPLES_8 | ((uint8_t)rate << 2) | HMC5883L_BIAS_NORMAL;
    if (write_register(HMC5883L_REG_CONFIG_A, config_a)) {
        g_mag.rate = rate;
    }
}

float magnetometer_get_theta(void) {
    return g_mag.data.theta;
}

float magnetometer_get_heading(void) {
    return g_mag.data.heading;
}

float magnetometer_get_magnitude(void) {
    return g_mag.data.magnitude;
}

void magnetometer_calibrate_start(void) {
    g_calibrating = true;
    g_cal_min[0] = g_cal_min[1] = g_cal_min[2] = 32767;
    g_cal_max[0] = g_cal_max[1] = g_cal_max[2] = -32768;
    g_cal_samples = 0;

    Serial.println("[MAG] Calibration started. Rotate device in figure-8 pattern.");
}

void magnetometer_calibrate_sample(void) {
    if (!g_calibrating) return;

    MagnetometerRaw raw = g_mag.raw;

    // Update min/max
    if (raw.x < g_cal_min[0]) g_cal_min[0] = raw.x;
    if (raw.y < g_cal_min[1]) g_cal_min[1] = raw.y;
    if (raw.z < g_cal_min[2]) g_cal_min[2] = raw.z;

    if (raw.x > g_cal_max[0]) g_cal_max[0] = raw.x;
    if (raw.y > g_cal_max[1]) g_cal_max[1] = raw.y;
    if (raw.z > g_cal_max[2]) g_cal_max[2] = raw.z;

    g_cal_samples++;
}

bool magnetometer_calibrate_finish(void) {
    if (!g_calibrating || g_cal_samples < 100) {
        Serial.println("[MAG] Calibration failed: insufficient samples");
        g_calibrating = false;
        return false;
    }

    g_calibrating = false;

    // Calculate hard iron offsets (center of min/max range)
    g_mag.calibration.offset_x = (g_cal_min[0] + g_cal_max[0]) / 2;
    g_mag.calibration.offset_y = (g_cal_min[1] + g_cal_max[1]) / 2;
    g_mag.calibration.offset_z = (g_cal_min[2] + g_cal_max[2]) / 2;

    // Calculate soft iron scale factors (normalize ranges)
    int16_t range_x = (g_cal_max[0] - g_cal_min[0]) / 2;
    int16_t range_y = (g_cal_max[1] - g_cal_min[1]) / 2;
    int16_t range_z = (g_cal_max[2] - g_cal_min[2]) / 2;

    int16_t avg_range = (range_x + range_y + range_z) / 3;

    if (avg_range > 0) {
        g_mag.calibration.scale_x = (int16_t)((float)avg_range / range_x * 1000.0f);
        g_mag.calibration.scale_y = (int16_t)((float)avg_range / range_y * 1000.0f);
        g_mag.calibration.scale_z = (int16_t)((float)avg_range / range_z * 1000.0f);
    }

    g_mag.calibration.calibrated = true;

    Serial.printf("[MAG] Calibration complete: offset(%d,%d,%d) scale(%d,%d,%d)\n",
        g_mag.calibration.offset_x, g_mag.calibration.offset_y, g_mag.calibration.offset_z,
        g_mag.calibration.scale_x, g_mag.calibration.scale_y, g_mag.calibration.scale_z);

    return true;
}

void magnetometer_set_declination(float declination) {
    g_mag.calibration.declination = declination;
}

bool magnetometer_load_calibration(void) {
    MagnetometerCalibration cal;

    EEPROM.begin(512);
    EEPROM.get(MAG_EEPROM_ADDR, cal);
    EEPROM.end();

    // Simple validity check
    if (cal.calibrated && abs(cal.scale_x) > 100 && abs(cal.scale_x) < 10000) {
        g_mag.calibration = cal;
        Serial.println("[MAG] Loaded calibration from EEPROM");
        return true;
    }

    return false;
}

bool magnetometer_save_calibration(void) {
    EEPROM.begin(512);
    EEPROM.put(MAG_EEPROM_ADDR, g_mag.calibration);
    bool success = EEPROM.commit();
    EEPROM.end();

    if (success) {
        Serial.println("[MAG] Calibration saved to EEPROM");
    }

    return success;
}

const MagnetometerState* magnetometer_get_state(void) {
    return &g_mag;
}

float magnetometer_modulate_coupling(float base_K) {
    if (!g_mag.data.valid) {
        return base_K;
    }

    // Modulate coupling based on field magnitude
    // Stronger/weaker field = adjusted coupling
    float field_ratio = g_mag.data.magnitude / MAG_FIELD_NOMINAL;

    // Clamp to reasonable range
    if (field_ratio < 0.5f) field_ratio = 0.5f;
    if (field_ratio > 2.0f) field_ratio = 2.0f;

    // Apply modulation using [R] range
    float modulation = 1.0f + MAG_COUPLING_RANGE * (field_ratio - 1.0f);

    return base_K * modulation;
}

bool magnetometer_is_healthy(void) {
    if (!g_mag.initialized || !g_mag.data.valid) {
        return false;
    }

    // Check field magnitude is reasonable (Earth's field ~25-65 μT)
    if (g_mag.data.magnitude < 10.0f || g_mag.data.magnitude > 100.0f) {
        return false;
    }

    // Check for data freshness
    if (millis() - g_mag.last_read_ms > 1000) {
        return false;
    }

    return true;
}

void magnetometer_reset(void) {
    if (!g_mag.initialized) return;

    // Soft reset by re-initializing
    g_mag.data_ready = false;

    // Configure device
    write_register(HMC5883L_REG_CONFIG_A,
                   HMC5883L_SAMPLES_8 | HMC5883L_RATE_15HZ | HMC5883L_BIAS_NORMAL);
    write_register(HMC5883L_REG_CONFIG_B, HMC5883L_GAIN_1_3);
    write_register(HMC5883L_REG_MODE, HMC5883L_MODE_CONT);

    g_heading_smoothed = 0.0f;
}

void magnetometer_print_diagnostics(void) {
    Serial.println("===============================================================");
    Serial.println("  MAGNETOMETER DIAGNOSTICS");
    Serial.println("===============================================================");
    Serial.printf("  Initialized: %s\n", g_mag.initialized ? "YES" : "NO");
    Serial.printf("  Healthy: %s\n", magnetometer_is_healthy() ? "YES" : "NO");
    Serial.printf("  Calibrated: %s\n", g_mag.calibration.calibrated ? "YES" : "NO");
    Serial.printf("  Read count: %lu\n", g_mag.read_count);
    Serial.printf("  Last read: %lu ms ago\n", millis() - g_mag.last_read_ms);
    Serial.println("  --- Raw Values ---");
    Serial.printf("  X: %d  Y: %d  Z: %d\n", g_mag.raw.x, g_mag.raw.y, g_mag.raw.z);
    Serial.println("  --- Calibrated Values ---");
    Serial.printf("  X: %.2f μT  Y: %.2f μT  Z: %.2f μT\n",
        g_mag.data.x, g_mag.data.y, g_mag.data.z);
    Serial.printf("  Magnitude: %.2f μT\n", g_mag.data.magnitude);
    Serial.printf("  Heading: %.1f°\n", g_mag.data.heading);
    Serial.printf("  Theta: %.4f rad\n", g_mag.data.theta);
    Serial.println("  --- Calibration ---");
    Serial.printf("  Offset: (%d, %d, %d)\n",
        g_mag.calibration.offset_x, g_mag.calibration.offset_y, g_mag.calibration.offset_z);
    Serial.printf("  Scale: (%d, %d, %d)\n",
        g_mag.calibration.scale_x, g_mag.calibration.scale_y, g_mag.calibration.scale_z);
    Serial.printf("  Declination: %.2f°\n", g_mag.calibration.declination);
    Serial.println("===============================================================");
}

#endif // UCF_V4_MODULES

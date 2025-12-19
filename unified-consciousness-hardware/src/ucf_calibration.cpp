/**
 * @file ucf_calibration.cpp
 * @brief UCF Lattice-Constrained Calibration v4.0.0
 *
 * Implements calibration routines that snap to lattice points.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "ucf/ucf_sacred_constants_v4.h"
#include "ucf/ucf_types.h"
#include "ucf/ucf_config.h"

// EEPROM address for calibration data
#define CALIBRATION_EEPROM_ADDR 0

// Global calibration data
static CalibrationData g_calibration;
static bool g_calibration_valid = false;

/**
 * @brief Compute CRC32 of a data buffer
 */
static uint32_t compute_crc32(const void* data, size_t length) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

/**
 * @brief Compute CRC32 of all sacred constants
 */
uint32_t compute_lattice_crc32(void) {
    float constants[] = {
        (float)PHI, (float)PHI_INV,
        (float)EULER, (float)EULER_INV,
        (float)UCF_PI, (float)PI_INV,
        (float)SQRT2, (float)SQRT2_INV,
        (float)LAMBDA_R_SQ, (float)LAMBDA_A_SQ,
        (float)Z_CRITICAL,
        (float)K_KAPPA_THRESHOLD, (float)K_ETA_THRESHOLD, (float)K_R_THRESHOLD,
        (float)TRIAD_HIGH, (float)TRIAD_LOW, (float)TRIAD_CROSSINGS
    };

    return compute_crc32(constants, sizeof(constants));
}

/**
 * @brief Load calibration data from EEPROM
 * @return true if calibration data is valid
 */
bool calibration_load(void) {
    EEPROM.begin(sizeof(CalibrationData) + 16);
    EEPROM.get(CALIBRATION_EEPROM_ADDR, g_calibration);
    EEPROM.end();

    // Check magic number
    if (g_calibration.magic != CALIBRATION_MAGIC) {
        Serial.println("[CAL] No valid calibration found (magic mismatch)");
        g_calibration_valid = false;
        return false;
    }

    // Verify CRC
    uint32_t expected_crc = compute_crc32(&g_calibration, sizeof(CalibrationData) - 4);
    if (expected_crc != g_calibration.crc) {
        Serial.println("[CAL] Calibration CRC mismatch");
        g_calibration_valid = false;
        return false;
    }

    // Verify lattice checksum
    if (g_calibration.lattice_checksum != compute_lattice_crc32()) {
        Serial.println("[CAL] Lattice checksum mismatch - constants may have changed");
        g_calibration_valid = false;
        return false;
    }

    Serial.printf("[CAL] Loaded calibration v%d\n", g_calibration.version);
    g_calibration_valid = true;
    return true;
}

/**
 * @brief Save calibration data to EEPROM
 * @return true if save successful
 */
bool calibration_save(void) {
    // Update magic and version
    g_calibration.magic = CALIBRATION_MAGIC;
    g_calibration.version = CALIBRATION_VERSION;

    // Update lattice checksum
    g_calibration.lattice_checksum = compute_lattice_crc32();

    // Compute CRC
    g_calibration.crc = compute_crc32(&g_calibration, sizeof(CalibrationData) - 4);

    // Write to EEPROM
    EEPROM.begin(sizeof(CalibrationData) + 16);
    EEPROM.put(CALIBRATION_EEPROM_ADDR, g_calibration);
    bool success = EEPROM.commit();
    EEPROM.end();

    if (success) {
        Serial.println("[CAL] Calibration saved");
        g_calibration_valid = true;
    } else {
        Serial.println("[CAL] Failed to save calibration");
    }

    return success;
}

/**
 * @brief Run factory calibration procedure
 */
void calibration_factory(void) {
    Serial.println("===============================================================");
    Serial.println("  UCF v4.0.0 FACTORY CALIBRATION");
    Serial.println("===============================================================");

    // Initialize calibration data
    memset(&g_calibration, 0, sizeof(CalibrationData));
    g_calibration.magic = CALIBRATION_MAGIC;
    g_calibration.version = CALIBRATION_VERSION;

    // 1. Sensor calibration - use lattice-derived thresholds
    Serial.println("  [1/4] Calibrating touch sensors...");
    for (int i = 0; i < HEX_SENSOR_COUNT; i++) {
        // Base threshold at PHI_INV ratio (lattice point [R])
        g_calibration.touch_threshold[i] = (uint16_t)(1024 * (1.0 - PHI_INV * 0.1));
        g_calibration.release_threshold[i] = (uint16_t)(1024 * (1.0 - PHI_INV * 0.05));

        Serial.printf("    Sensor %d: touch=%d release=%d\n",
            i, g_calibration.touch_threshold[i], g_calibration.release_threshold[i]);
    }

    // 2. Magnetometer calibration - default offsets
    Serial.println("  [2/4] Setting magnetometer defaults...");
    g_calibration.mag_offset[0] = 0;
    g_calibration.mag_offset[1] = 0;
    g_calibration.mag_offset[2] = 0;
    g_calibration.mag_scale[0] = 1.0f;
    g_calibration.mag_scale[1] = 1.0f;
    g_calibration.mag_scale[2] = 1.0f;

    // 3. Kuramoto natural frequencies - lattice-derived
    Serial.println("  [3/4] Calibrating oscillator frequencies...");
    for (int i = 0; i < N_OSCILLATORS; i++) {
        // Natural frequencies around EULER_INV with PHI_INV spread
        g_calibration.natural_frequencies[i] = EULER_INV + (i - N_OSCILLATORS/2) * 0.05 * PHI_INV;
        Serial.printf("    Oscillator %d: freq=%.4f\n", i, g_calibration.natural_frequencies[i]);
    }
    g_calibration.coupling_adjustment = 1.0;

    // 4. Compute and store lattice checksum
    Serial.println("  [4/4] Computing lattice checksum...");
    g_calibration.lattice_checksum = compute_lattice_crc32();

    // Save to EEPROM
    calibration_save();

    Serial.println("===============================================================");
    Serial.println("  CALIBRATION COMPLETE");
    Serial.printf("  Lattice CRC: 0x%08X\n", g_calibration.lattice_checksum);
    Serial.println("===============================================================");
}

/**
 * @brief Calibrate a single sensor threshold using lattice snapping
 * @param sensor_index Sensor index (0-18)
 * @param raw_value Raw sensor reading
 */
void calibration_snap_sensor(uint8_t sensor_index, uint16_t raw_value) {
    if (sensor_index >= HEX_SENSOR_COUNT) return;

    // Normalize to [0, 1]
    double normalized = (double)raw_value / 4095.0;

    // Snap to nearest lattice point
    double snapped = snap_to_lattice(normalized);

    // Store calibrated value
    g_calibration.touch_threshold[sensor_index] = (uint16_t)(snapped * 4095.0);

    Serial.printf("[CAL] Sensor %d: %.4f -> %.4f (lattice)\n",
        sensor_index, normalized, snapped);
}

/**
 * @brief Get calibration data pointer
 */
CalibrationData* calibration_get(void) {
    return &g_calibration;
}

/**
 * @brief Check if calibration is valid
 */
bool calibration_is_valid(void) {
    return g_calibration_valid;
}

/**
 * @brief Reset calibration to defaults
 */
void calibration_reset(void) {
    g_calibration_valid = false;
    calibration_factory();
}

/**
 * @brief Print calibration info
 */
void calibration_print(void) {
    Serial.println("===============================================================");
    Serial.println("  CALIBRATION DATA");
    Serial.println("===============================================================");
    Serial.printf("  Magic: 0x%08X (%s)\n",
        g_calibration.magic,
        g_calibration.magic == CALIBRATION_MAGIC ? "valid" : "invalid");
    Serial.printf("  Version: %d\n", g_calibration.version);
    Serial.printf("  Lattice CRC: 0x%08X\n", g_calibration.lattice_checksum);
    Serial.printf("  Data CRC: 0x%08X\n", g_calibration.crc);
    Serial.printf("  Valid: %s\n", g_calibration_valid ? "YES" : "NO");
    Serial.println("===============================================================");
}

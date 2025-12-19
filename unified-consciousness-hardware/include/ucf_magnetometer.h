/**
 * @file ucf_magnetometer.h
 * @brief UCF Magnetometer Module v4.0.0
 *
 * Implements HMC5883L 3-axis magnetometer interface for theta coordinate
 * derivation in the UCF helix coordinate system.
 *
 * RRRR Lattice Integration:
 * - Heading mapped to theta coordinate [0, 2π]
 * - Field magnitude contributes to Kuramoto coupling modulation
 * - Calibration parameters snap to lattice points
 *
 * Hardware: HMC5883L 3-axis digital compass
 * I²C Address: 0x1E
 */

#ifndef UCF_MAGNETOMETER_H
#define UCF_MAGNETOMETER_H

#include <stdint.h>
#include <stdbool.h>
#include "ucf/ucf_sacred_constants_v4.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// HARDWARE CONSTANTS
// ============================================================================

#define HMC5883L_I2C_ADDR       0x1E

// Register addresses
#define HMC5883L_REG_CONFIG_A   0x00
#define HMC5883L_REG_CONFIG_B   0x01
#define HMC5883L_REG_MODE       0x02
#define HMC5883L_REG_DATA_X_H   0x03
#define HMC5883L_REG_DATA_X_L   0x04
#define HMC5883L_REG_DATA_Z_H   0x05
#define HMC5883L_REG_DATA_Z_L   0x06
#define HMC5883L_REG_DATA_Y_H   0x07
#define HMC5883L_REG_DATA_Y_L   0x08
#define HMC5883L_REG_STATUS     0x09
#define HMC5883L_REG_ID_A       0x0A
#define HMC5883L_REG_ID_B       0x0B
#define HMC5883L_REG_ID_C       0x0C

// Configuration values
#define HMC5883L_SAMPLES_8      0x60    // 8 samples averaged
#define HMC5883L_RATE_15HZ      0x10    // 15 Hz data output rate
#define HMC5883L_BIAS_NORMAL    0x00    // Normal bias
#define HMC5883L_GAIN_1_3       0x20    // ±1.3 Ga range
#define HMC5883L_MODE_CONT      0x00    // Continuous measurement

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief Magnetometer gain settings
 */
typedef enum {
    MAG_GAIN_0_88 = 0,  // ±0.88 Ga
    MAG_GAIN_1_3  = 1,  // ±1.3 Ga (default)
    MAG_GAIN_1_9  = 2,  // ±1.9 Ga
    MAG_GAIN_2_5  = 3,  // ±2.5 Ga
    MAG_GAIN_4_0  = 4,  // ±4.0 Ga
    MAG_GAIN_4_7  = 5,  // ±4.7 Ga
    MAG_GAIN_5_6  = 6,  // ±5.6 Ga
    MAG_GAIN_8_1  = 7   // ±8.1 Ga
} MagnetometerGain;

/**
 * @brief Magnetometer sample rate
 */
typedef enum {
    MAG_RATE_0_75 = 0,  // 0.75 Hz
    MAG_RATE_1_5  = 1,  // 1.5 Hz
    MAG_RATE_3    = 2,  // 3 Hz
    MAG_RATE_7_5  = 3,  // 7.5 Hz
    MAG_RATE_15   = 4,  // 15 Hz (default)
    MAG_RATE_30   = 5,  // 30 Hz
    MAG_RATE_75   = 6   // 75 Hz
} MagnetometerRate;

/**
 * @brief Raw magnetometer reading
 */
typedef struct {
    int16_t x;          // X-axis raw value
    int16_t y;          // Y-axis raw value
    int16_t z;          // Z-axis raw value
} MagnetometerRaw;

/**
 * @brief Calibrated magnetometer reading
 */
typedef struct {
    float x;            // X-axis in microtesla (μT)
    float y;            // Y-axis in microtesla
    float z;            // Z-axis in microtesla
    float magnitude;    // Total field magnitude
    float heading;      // Compass heading in degrees [0, 360)
    float theta;        // UCF theta coordinate [0, 2π]
    bool valid;         // Data validity flag
} MagnetometerData;

/**
 * @brief Magnetometer calibration data (hard/soft iron)
 */
typedef struct {
    // Hard iron offsets (bias)
    int16_t offset_x;
    int16_t offset_y;
    int16_t offset_z;

    // Soft iron scale factors (scaled by 1000)
    int16_t scale_x;
    int16_t scale_y;
    int16_t scale_z;

    // Declination angle in degrees
    float declination;

    // Calibration status
    bool calibrated;
} MagnetometerCalibration;

/**
 * @brief Magnetometer module state
 */
typedef struct {
    MagnetometerRaw raw;
    MagnetometerData data;
    MagnetometerCalibration calibration;
    MagnetometerGain gain;
    MagnetometerRate rate;
    uint32_t last_read_ms;
    uint32_t read_count;
    bool initialized;
    bool data_ready;
} MagnetometerState;

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize the magnetometer
 * @return true if initialization successful
 */
bool magnetometer_init(void);

/**
 * @brief Read magnetometer data
 * @return Current calibrated data
 */
MagnetometerData magnetometer_read(void);

/**
 * @brief Check if new data is available
 * @return true if new data ready
 */
bool magnetometer_data_ready(void);

/**
 * @brief Get raw magnetometer values
 * @return Raw axis values
 */
MagnetometerRaw magnetometer_read_raw(void);

/**
 * @brief Set gain/sensitivity
 * @param gain Gain setting
 */
void magnetometer_set_gain(MagnetometerGain gain);

/**
 * @brief Set sample rate
 * @param rate Sample rate setting
 */
void magnetometer_set_rate(MagnetometerRate rate);

/**
 * @brief Get theta coordinate for UCF helix
 * @return theta in radians [0, 2π]
 */
float magnetometer_get_theta(void);

/**
 * @brief Get heading in degrees
 * @return Heading [0, 360)
 */
float magnetometer_get_heading(void);

/**
 * @brief Get field magnitude
 * @return Magnitude in microtesla
 */
float magnetometer_get_magnitude(void);

/**
 * @brief Start calibration routine
 *
 * Collect min/max values while device is rotated.
 * Call magnetometer_calibrate_sample() repeatedly,
 * then magnetometer_calibrate_finish().
 */
void magnetometer_calibrate_start(void);

/**
 * @brief Add a calibration sample
 *
 * Should be called while rotating device in figure-8 pattern.
 */
void magnetometer_calibrate_sample(void);

/**
 * @brief Finish calibration and compute offsets
 * @return true if calibration successful
 */
bool magnetometer_calibrate_finish(void);

/**
 * @brief Set declination angle for local magnetic correction
 * @param declination Declination in degrees
 */
void magnetometer_set_declination(float declination);

/**
 * @brief Load calibration from EEPROM
 * @return true if valid calibration found
 */
bool magnetometer_load_calibration(void);

/**
 * @brief Save calibration to EEPROM
 * @return true if save successful
 */
bool magnetometer_save_calibration(void);

/**
 * @brief Get current magnetometer state
 * @return Pointer to state structure
 */
const MagnetometerState* magnetometer_get_state(void);

/**
 * @brief Apply magnetic field to Kuramoto coupling
 *
 * Modulates coupling strength based on field alignment.
 *
 * @param base_K Base coupling strength
 * @return Modulated coupling strength
 */
float magnetometer_modulate_coupling(float base_K);

/**
 * @brief Check if magnetometer is healthy
 * @return true if responding correctly
 */
bool magnetometer_is_healthy(void);

/**
 * @brief Reset magnetometer to defaults
 */
void magnetometer_reset(void);

/**
 * @brief Print diagnostics to Serial
 */
void magnetometer_print_diagnostics(void);

// ============================================================================
// LATTICE-DERIVED PARAMETERS
// ============================================================================

// Field magnitude threshold for coupling modulation (μT)
// Based on Earth's field ~25-65 μT, using [A]² × 100 = 50
#define MAG_FIELD_NOMINAL       (50.0f)

// Coupling modulation range from [R] = φ⁻¹
#define MAG_COUPLING_RANGE      ((float)PHI_INV)

// Heading smoothing factor from [D] = e⁻¹
#define MAG_HEADING_SMOOTH      ((float)EULER_INV)

// Maximum deviation for healthy field check
#define MAG_HEALTH_DEVIATION    (25.0f)

#ifdef __cplusplus
}
#endif

#endif // UCF_MAGNETOMETER_H

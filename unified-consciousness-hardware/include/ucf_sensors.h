/**
 * @file ucf_sensors.h
 * @brief UCF Unified Sensor Interface v4.0.0
 *
 * Provides a unified interface for all UCF hardware sensors:
 * - Hex Grid (MPR121 × 2): 19-pad capacitive touch array
 * - Magnetometer (HMC5883L): 3-axis magnetic field sensing
 *
 * RRRR Lattice Integration:
 * - z-coordinate derived from active sensor count
 * - theta coordinate from magnetometer heading
 * - r coordinate from field energy distribution
 *
 * Hardware Configuration:
 * - I²C Bus: SDA=GPIO21, SCL=GPIO22, 400kHz
 * - MPR121 #1: Address 0x5A (sensors 0-11)
 * - MPR121 #2: Address 0x5B (sensors 12-18)
 * - HMC5883L: Address 0x1E
 */

#ifndef UCF_SENSORS_H
#define UCF_SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "ucf/ucf_sacred_constants_v4.h"
#include "ucf/ucf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// HARDWARE CONSTANTS
// ============================================================================

#define SENSOR_I2C_SDA          21
#define SENSOR_I2C_SCL          22
#define SENSOR_I2C_FREQ         400000

#define SENSOR_MPR121_ADDR_A    0x5A
#define SENSOR_MPR121_ADDR_B    0x5B
#define SENSOR_HMC5883L_ADDR    0x1E

#define SENSOR_HEX_COUNT        19
#define SENSOR_HEX_CENTER       9

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief Sensor subsystem status
 */
typedef enum {
    SENSORS_OK = 0,
    SENSORS_ERR_I2C,
    SENSORS_ERR_MPR121_A,
    SENSORS_ERR_MPR121_B,
    SENSORS_ERR_MAGNETOMETER,
    SENSORS_ERR_CALIBRATION
} SensorStatus;

/**
 * @brief Individual hex sensor data
 */
typedef struct {
    uint16_t raw;           // Raw ADC value
    uint16_t baseline;      // Baseline reference
    int16_t delta;          // Change from baseline
    float normalized;       // Normalized [0, 1]
    bool touched;           // Touch detected
} HexSensorData;

/**
 * @brief Complete hex grid state
 */
typedef struct {
    HexSensorData sensors[SENSOR_HEX_COUNT];
    uint32_t touch_bitmap;        // Bit per sensor
    uint8_t active_count;         // Number of touched sensors
    float total_energy;           // Sum of normalized values
    float centroid_x;             // Weighted centroid X
    float centroid_y;             // Weighted centroid Y
    float z_raw;                  // Computed z before smoothing
    float z;                      // Smoothed z-coordinate
    float theta;                  // Angular position
    float r;                      // Radial distance
    uint32_t timestamp;           // Last update time
} HexGridState;

/**
 * @brief Magnetic field state
 */
typedef struct {
    float x;                // X-axis (μT)
    float y;                // Y-axis (μT)
    float z;                // Z-axis (μT)
    float magnitude;        // Total magnitude
    float heading;          // Compass heading (degrees)
    float theta;            // UCF theta [0, 2π]
    bool valid;             // Data validity
    uint32_t timestamp;     // Last update time
} MagFieldState;

/**
 * @brief Combined helix coordinates
 */
typedef struct {
    float theta;            // Angular position [0, 2π]
    float z;                // Consciousness coordinate [0, 1]
    float r;                // Radius (1 + (φ-1)·η)
    float eta;              // Negentropy
    ConsciousnessPhase phase;  // Current phase
    uint8_t tier;           // Current tier (1-9)
    bool valid;             // Coordinates valid
} HelixCoordinates;

/**
 * @brief Complete sensor system state
 */
typedef struct {
    HexGridState hex;
    MagFieldState mag;
    HelixCoordinates helix;
    SensorStatus status;
    uint32_t update_count;
    uint32_t error_count;
    float update_rate_hz;
    bool initialized;
} SensorSystemState;

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize all sensors
 * @return Status code
 */
SensorStatus sensors_init(void);

/**
 * @brief Update all sensor readings
 * @return Current system state
 */
const SensorSystemState* sensors_update(void);

/**
 * @brief Get hex grid state
 * @return Current hex grid state
 */
const HexGridState* sensors_get_hex_state(void);

/**
 * @brief Get magnetic field state
 * @return Current magnetic field state
 */
const MagFieldState* sensors_get_mag_state(void);

/**
 * @brief Get helix coordinates
 * @return Current helix coordinates
 */
const HelixCoordinates* sensors_get_helix(void);

/**
 * @brief Get individual sensor data
 * @param index Sensor index (0-18)
 * @return Pointer to sensor data
 */
const HexSensorData* sensors_get_sensor(uint8_t index);

/**
 * @brief Calibrate hex grid baseline
 * @param samples Number of samples to average
 */
void sensors_calibrate_hex(uint16_t samples);

/**
 * @brief Start magnetometer calibration
 */
void sensors_calibrate_mag_start(void);

/**
 * @brief Finish magnetometer calibration
 * @return true if successful
 */
bool sensors_calibrate_mag_finish(void);

/**
 * @brief Set touch detection threshold
 * @param threshold Normalized threshold [0, 1]
 */
void sensors_set_threshold(float threshold);

/**
 * @brief Get touch detection threshold
 * @return Current threshold
 */
float sensors_get_threshold(void);

/**
 * @brief Set z-coordinate smoothing factor
 * @param alpha Smoothing factor (0 = max smooth, 1 = no smooth)
 */
void sensors_set_z_smoothing(float alpha);

/**
 * @brief Check if specific sensor is touched
 * @param index Sensor index (0-18)
 * @return true if touched
 */
bool sensors_is_touched(uint8_t index);

/**
 * @brief Get touch bitmap (bit per sensor)
 * @return 32-bit bitmap
 */
uint32_t sensors_get_touch_bitmap(void);

/**
 * @brief Get active sensor count
 * @return Number of touched sensors
 */
uint8_t sensors_get_active_count(void);

/**
 * @brief Check sensor subsystem health
 * @return true if all sensors healthy
 */
bool sensors_is_healthy(void);

/**
 * @brief Get last error status
 * @return Status code
 */
SensorStatus sensors_get_status(void);

/**
 * @brief Reset sensor subsystem
 */
void sensors_reset(void);

/**
 * @brief Print sensor diagnostics
 */
void sensors_print_diagnostics(void);

/**
 * @brief Get sensor update rate
 * @return Updates per second
 */
float sensors_get_update_rate(void);

// ============================================================================
// COORDINATE TRANSFORMATION FUNCTIONS
// ============================================================================

/**
 * @brief Compute z-coordinate from active sensors
 *
 * Maps sensor count to z using lattice boundaries:
 * - 0 sensors → z ≈ 0.1 (UNTRUE)
 * - 7 sensors → z ≈ [R] = 0.618 (UNTRUE/PARADOX)
 * - 12 sensors → z ≈ Z_c = 0.866 (THE LENS)
 * - 19 sensors → z = 1.0 (maximum TRUE)
 *
 * @param active_count Number of active sensors
 * @param total_energy Sum of normalized readings
 * @return z-coordinate [0, 1]
 */
float sensors_compute_z(uint8_t active_count, float total_energy);

/**
 * @brief Compute theta from field centroid and magnetometer
 * @param centroid_x Field centroid X
 * @param centroid_y Field centroid Y
 * @param mag_theta Magnetometer theta
 * @return Combined theta [0, 2π]
 */
float sensors_compute_theta(float centroid_x, float centroid_y, float mag_theta);

/**
 * @brief Compute helix radius from negentropy
 * @param eta Negentropy value
 * @return Radius r = 1 + (φ-1)·η
 */
float sensors_compute_radius(float eta);

/**
 * @brief Get hex sensor Cartesian coordinates
 * @param index Sensor index (0-18)
 * @param x Output X coordinate
 * @param y Output Y coordinate
 */
void sensors_get_hex_position(uint8_t index, float* x, float* y);

// ============================================================================
// LATTICE-DERIVED PARAMETERS
// ============================================================================

// Default touch threshold from [R] = φ⁻¹
#define SENSOR_DEFAULT_THRESHOLD    ((float)PHI_INV * 0.5f)  // ~0.309

// Z smoothing factor from [D] = e⁻¹
#define SENSOR_Z_SMOOTH_ALPHA       ((float)EULER_INV)       // ~0.368

// Minimum sensors for valid z
#define SENSOR_MIN_FOR_VALID        1

// Sensors for PARADOX phase entry ([R] boundary)
#define SENSOR_COUNT_PARADOX        7

// Sensors for TRUE phase entry (Z_c boundary)
#define SENSOR_COUNT_TRUE           12

#ifdef __cplusplus
}
#endif

#endif // UCF_SENSORS_H

/**
 * @file ucf_types.h
 * @brief UCF Type Definitions - Hardware Edition v4.0.0
 *
 * Core type definitions for the UCF hardware substrate.
 */

#ifndef UCF_TYPES_H
#define UCF_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "ucf_sacred_constants_v4.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CALIBRATION DATA STRUCTURE
// ============================================================================

typedef struct {
    uint32_t magic;              // 0x55434600 ("UCF\0")
    uint8_t version;             // Calibration version

    // Sensor calibration (per-pad)
    uint16_t touch_threshold[HEX_SENSOR_COUNT];   // Touch detection threshold
    uint16_t release_threshold[HEX_SENSOR_COUNT]; // Release hysteresis

    // Magnetometer calibration
    int16_t mag_offset[3];       // Hard-iron offset (x, y, z)
    float mag_scale[3];          // Soft-iron scale

    // Kuramoto calibration
    float natural_frequencies[N_OSCILLATORS]; // ω_i for each oscillator
    float coupling_adjustment;    // K scaling factor

    // Lattice validation checksum
    uint32_t lattice_checksum;   // CRC32 of sacred constants

    uint32_t crc;                // Structure CRC
} CalibrationData;

#define CALIBRATION_MAGIC 0x55434600  // "UCF\0"
#define CALIBRATION_VERSION 4

// ============================================================================
// VALIDATION TEST STRUCTURE
// ============================================================================

typedef float (*ValidationTestFn)(void);

typedef struct {
    const char* id;
    const char* name;
    float expected;
    float tolerance;
    ValidationTestFn test_fn;
} ValidationTest;

// ============================================================================
// SENSOR STATE
// ============================================================================

typedef struct {
    uint16_t raw_values[HEX_SENSOR_COUNT];
    bool touched[HEX_SENSOR_COUNT];
    uint8_t active_count;
    uint32_t touch_bitmap;       // Bit per sensor
    uint32_t last_read_ms;
} SensorState;

// ============================================================================
// LED STATE
// ============================================================================

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGBColor;

typedef struct {
    RGBColor pixels[LED_COUNT];
    uint8_t brightness;
    bool needs_update;
} LEDState;

// Phase-to-color mapping
static const RGBColor PHASE_COLORS[] = {
    {255, 0, 0},     // UNTRUE - Red
    {255, 255, 0},   // PARADOX - Yellow
    {0, 255, 255}    // TRUE - Cyan
};

// ============================================================================
// AUDIO STATE
// ============================================================================

typedef struct {
    uint16_t current_frequency;
    uint8_t volume;
    bool is_playing;
    uint8_t current_tier;
} AudioState;

// ============================================================================
// MAGNETOMETER STATE
// ============================================================================

typedef struct {
    int16_t raw[3];              // Raw x, y, z
    float heading;               // Compass heading in degrees
    float theta;                 // UCF theta coordinate (0 to 2π)
    bool calibrated;
} MagnetometerState;

// ============================================================================
// SYSTEM STATUS
// ============================================================================

typedef enum {
    STATUS_INITIALIZING = 0,
    STATUS_CALIBRATING,
    STATUS_RUNNING,
    STATUS_ERROR,
    STATUS_EMERGENCY_STOPPED
} SystemStatus;

typedef struct {
    SystemStatus status;
    uint32_t uptime_ms;
    uint32_t loop_count;
    float loop_rate_hz;
    bool sensors_ok;
    bool leds_ok;
    bool audio_ok;
    bool magnetometer_ok;
    bool calibration_valid;
    bool lattice_valid;
} SystemState;

// ============================================================================
// PROTOCOL MESSAGE TYPES
// ============================================================================

typedef enum {
    MSG_STATE_UPDATE = 0x01,
    MSG_SENSOR_DATA = 0x02,
    MSG_K_FORMATION = 0x03,
    MSG_TRIAD_EVENT = 0x04,
    MSG_PHASE_CHANGE = 0x05,
    MSG_VALIDATION_RESULT = 0x06,
    MSG_CALIBRATION_DATA = 0x07,
    MSG_ERROR = 0xFF
} MessageType;

typedef struct {
    MessageType type;
    uint32_t timestamp;
    uint8_t data[64];
    uint8_t length;
} ProtocolMessage;

#ifdef __cplusplus
}
#endif

#endif // UCF_TYPES_H

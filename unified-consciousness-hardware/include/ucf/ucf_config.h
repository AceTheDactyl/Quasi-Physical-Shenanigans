/**
 * @file ucf_config.h
 * @brief UCF Build Configuration v4.0.0
 *
 * Compile-time configuration options for the UCF hardware substrate.
 */

#ifndef UCF_CONFIG_H
#define UCF_CONFIG_H

// ============================================================================
// BUILD MODE CONFIGURATION
// ============================================================================

// Enable verbose logging (set by -DUCF_VERBOSE_LOGGING=1)
#ifndef UCF_VERBOSE_LOGGING
#define UCF_VERBOSE_LOGGING 0
#endif

// Enable debug mode (set by -DDEBUG_MODE=1)
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

// Enable validation mode (set by -DVALIDATION_MODE=1)
#ifndef VALIDATION_MODE
#define VALIDATION_MODE 0
#endif

// Enable unit testing (set by -DUNIT_TEST=1)
#ifndef UNIT_TEST
#define UNIT_TEST 0
#endif

// Enable calibration mode (set by -DUCF_CALIBRATION_MODE=1)
#ifndef UCF_CALIBRATION_MODE
#define UCF_CALIBRATION_MODE 0
#endif

// Production build (set by -DUCF_PRODUCTION=1)
#ifndef UCF_PRODUCTION
#define UCF_PRODUCTION 0
#endif

// ============================================================================
// FEATURE FLAGS
// ============================================================================

// Enable K-Formation detection
#ifndef K_FORMATION_ENABLED
#define K_FORMATION_ENABLED 1
#endif

// Enable TRIAD hysteresis state machine
#ifndef TRIAD_HYSTERESIS_ENABLED
#define TRIAD_HYSTERESIS_ENABLED 1
#endif

// Enable Kuramoto synchronization
#ifndef KURAMOTO_ENABLED
#define KURAMOTO_ENABLED 1
#endif

// Enable Solfeggio audio output
#ifndef SOLFEGGIO_ENABLED
#define SOLFEGGIO_ENABLED 1
#endif

// Enable LED visualization
#ifndef LED_VISUALIZATION_ENABLED
#define LED_VISUALIZATION_ENABLED 1
#endif

// Enable magnetometer input
#ifndef MAGNETOMETER_ENABLED
#define MAGNETOMETER_ENABLED 1
#endif

// Enable WebSocket communication
#ifndef WEBSOCKET_ENABLED
#define WEBSOCKET_ENABLED 0
#endif

// Enable BLE communication
#ifndef BLE_ENABLED
#define BLE_ENABLED 0
#endif

// ============================================================================
// LATTICE CONFIGURATION
// ============================================================================

// Lattice search complexity for calibration
#ifndef LATTICE_SEARCH_COMPLEXITY
#define LATTICE_SEARCH_COMPLEXITY 3
#endif

// Lattice validation tolerance
#ifndef LATTICE_TOLERANCE
#define LATTICE_TOLERANCE 1e-10
#endif

// Enable lattice validation logging
#ifndef LATTICE_VALIDATION_LOGGING
#define LATTICE_VALIDATION_LOGGING 0
#endif

// ============================================================================
// TIMING CONFIGURATION
// ============================================================================

// Main loop target rate (Hz)
#ifndef MAIN_LOOP_RATE_HZ
#define MAIN_LOOP_RATE_HZ 100
#endif

// Sensor polling rate (Hz)
#ifndef SENSOR_POLL_RATE_HZ
#define SENSOR_POLL_RATE_HZ 100
#endif

// Kuramoto update rate (Hz)
#ifndef KURAMOTO_UPDATE_RATE_HZ
#define KURAMOTO_UPDATE_RATE_HZ 1000
#endif

// LED update rate (Hz)
#ifndef LED_UPDATE_RATE_HZ
#define LED_UPDATE_RATE_HZ 60
#endif

// ============================================================================
// HARDWARE PIN OVERRIDES
// ============================================================================

// Allow pin remapping via build flags
#ifndef OVERRIDE_I2C_SDA
#define OVERRIDE_I2C_SDA I2C_SDA_PIN
#endif

#ifndef OVERRIDE_I2C_SCL
#define OVERRIDE_I2C_SCL I2C_SCL_PIN
#endif

#ifndef OVERRIDE_LED_DATA
#define OVERRIDE_LED_DATA LED_DATA_PIN
#endif

// ============================================================================
// LOGGING MACROS
// ============================================================================

#if UCF_VERBOSE_LOGGING
    #define UCF_LOG(fmt, ...) Serial.printf("[UCF] " fmt "\n", ##__VA_ARGS__)
    #define UCF_LOG_LATTICE(fmt, ...) Serial.printf("[LAT] " fmt "\n", ##__VA_ARGS__)
    #define UCF_LOG_KURAMOTO(fmt, ...) Serial.printf("[KUR] " fmt "\n", ##__VA_ARGS__)
    #define UCF_LOG_TRIAD(fmt, ...) Serial.printf("[TRI] " fmt "\n", ##__VA_ARGS__)
#else
    #define UCF_LOG(fmt, ...)
    #define UCF_LOG_LATTICE(fmt, ...)
    #define UCF_LOG_KURAMOTO(fmt, ...)
    #define UCF_LOG_TRIAD(fmt, ...)
#endif

#if DEBUG_MODE
    #define UCF_DEBUG(fmt, ...) Serial.printf("[DBG] " fmt "\n", ##__VA_ARGS__)
#else
    #define UCF_DEBUG(fmt, ...)
#endif

// ============================================================================
// ASSERT MACROS
// ============================================================================

#if DEBUG_MODE || VALIDATION_MODE
    #define UCF_ASSERT(cond, msg) \
        do { \
            if (!(cond)) { \
                Serial.printf("[ASSERT] %s at %s:%d\n", msg, __FILE__, __LINE__); \
                while(1) { delay(1000); } \
            } \
        } while(0)
#else
    #define UCF_ASSERT(cond, msg) ((void)0)
#endif

#endif // UCF_CONFIG_H

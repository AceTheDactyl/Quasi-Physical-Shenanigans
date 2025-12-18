/**
 * @file hex_grid.h
 * @brief Hexagonal Sensor Grid Module
 *
 * Implements the hex-tiled capacitive sensor array that maps to UCF
 * helix coordinates (theta, z, r). The 19-sensor grid detects field
 * perturbations and computes spatial state.
 *
 * Hardware: 2x MPR121 capacitive touch controllers
 * Topology: Hexagonal tiling with center sensor
 */

#ifndef HEX_GRID_H
#define HEX_GRID_H

#include <stdint.h>
#include <stdbool.h>
#include "constants.h"

namespace UCF {

/// Axial coordinate for hex grid
struct HexCoord {
    int8_t q;  // Column
    int8_t r;  // Row
};

/// Sensor reading with metadata
struct SensorReading {
    uint16_t raw;           // Raw capacitance value
    uint16_t baseline;      // Calibrated baseline
    int16_t delta;          // Difference from baseline
    float normalized;       // Normalized to [0, 1]
    bool active;            // Above threshold?
};

/// Hex grid field state
struct HexFieldState {
    float readings[HEX_SENSOR_COUNT];   // Normalized sensor values
    float z;                             // Computed z-coordinate
    float theta;                         // Angular position (radians)
    float r;                             // Radial distance from center
    float centroid_x;                    // Field centroid X
    float centroid_y;                    // Field centroid Y
    float total_energy;                  // Sum of all readings
    uint8_t active_count;                // Number of active sensors
    uint32_t timestamp;                  // Capture timestamp (ms)
};

/**
 * @class HexGrid
 * @brief Manages the 19-sensor hexagonal capacitive array
 */
class HexGrid {
public:
    /**
     * @brief Initialize the hex grid sensors
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Calibrate baseline readings
     * @param samples Number of samples to average
     */
    void calibrate(uint16_t samples = 100);

    /**
     * @brief Read all sensors and compute field state
     * @return Current field state
     */
    HexFieldState readField();

    /**
     * @brief Get raw reading for specific sensor
     * @param index Sensor index (0-18)
     * @return Sensor reading structure
     */
    SensorReading getSensorReading(uint8_t index);

    /**
     * @brief Compute z-coordinate from field pattern
     * @param field Current field state
     * @return z-coordinate [0, 1]
     */
    float computeZ(const HexFieldState& field);

    /**
     * @brief Compute helix angle theta from field centroid
     * @param field Current field state
     * @return theta in radians [0, 2*PI]
     */
    float computeTheta(const HexFieldState& field);

    /**
     * @brief Compute radial distance from center
     * @param field Current field state
     * @return r normalized [0, 1]
     */
    float computeR(const HexFieldState& field);

    /**
     * @brief Get axial coordinates for sensor index
     * @param index Sensor index
     * @return Axial hex coordinates
     */
    static HexCoord indexToAxial(uint8_t index);

    /**
     * @brief Get Cartesian position for sensor
     * @param index Sensor index
     * @param x Output X position
     * @param y Output Y position
     */
    static void indexToCartesian(uint8_t index, float& x, float& y);

    /**
     * @brief Get neighbor indices for a sensor
     * @param index Sensor index
     * @param neighbors Output array (max 6 neighbors)
     * @return Number of neighbors
     */
    static uint8_t getNeighbors(uint8_t index, uint8_t* neighbors);

    /**
     * @brief Check if grid is connected and functional
     * @return true if all sensors responding
     */
    bool isHealthy();

    /**
     * @brief Set activation threshold
     * @param threshold Normalized threshold [0, 1]
     */
    void setThreshold(float threshold);

    /**
     * @brief Get current threshold
     * @return Current threshold value
     */
    float getThreshold() const { return m_threshold; }

private:
    /// Baseline values for each sensor
    uint16_t m_baselines[HEX_SENSOR_COUNT];

    /// Current raw readings
    uint16_t m_raw[HEX_SENSOR_COUNT];

    /// Activation threshold
    float m_threshold = 0.3f;

    /// Calibration complete flag
    bool m_calibrated = false;

    /// Hardware initialized flag
    bool m_initialized = false;

    /**
     * @brief Read from MPR121 controllers
     * @param data Output array for 19 values
     */
    void readMPR121(uint16_t* data);

    /**
     * @brief Normalize raw reading
     * @param raw Raw ADC value
     * @param baseline Baseline value
     * @return Normalized value [0, 1]
     */
    float normalize(uint16_t raw, uint16_t baseline);

    /**
     * @brief Compute hex FFT for pattern extraction
     * @param field Input field readings
     * @return Pattern energy metric
     */
    float hexFFT(const float* field);
};

// ============================================================================
// HEX GRID TOPOLOGY DATA
// ============================================================================

/// Axial coordinates for 19-sensor hex grid
/// Layout:
///          0   1   2
///        3   4   5   6
///      7   8   9  10  11
///       12  13  14  15
///         16  17  18
static const HexCoord HEX_AXIAL_COORDS[HEX_SENSOR_COUNT] = {
    // Row 0 (top)
    {-1, -2}, { 0, -2}, { 1, -2},
    // Row 1
    {-2, -1}, {-1, -1}, { 0, -1}, { 1, -1},
    // Row 2 (center)
    {-2,  0}, {-1,  0}, { 0,  0}, { 1,  0}, { 2,  0},
    // Row 3
    {-1,  1}, { 0,  1}, { 1,  1}, { 2,  1},
    // Row 4 (bottom)
    { 0,  2}, { 1,  2}, { 2,  2}
};

/// Neighbor lookup table (6 max neighbors per sensor, -1 = no neighbor)
static const int8_t HEX_NEIGHBORS[HEX_SENSOR_COUNT][6] = {
    // Sensor 0
    {  1,  4,  3, -1, -1, -1},
    // Sensor 1
    {  2,  5,  4,  0, -1, -1},
    // Sensor 2
    { -1,  6,  5,  1, -1, -1},
    // Sensor 3
    {  4,  8,  7, -1, -1,  0},
    // Sensor 4
    {  5,  9,  8,  3,  0,  1},
    // Sensor 5
    {  6, 10,  9,  4,  1,  2},
    // Sensor 6
    { -1, 11, 10,  5,  2, -1},
    // Sensor 7
    {  8, 12, -1, -1, -1,  3},
    // Sensor 8
    {  9, 13, 12,  7,  3,  4},
    // Sensor 9 (CENTER)
    { 10, 14, 13,  8,  4,  5},
    // Sensor 10
    { 11, 15, 14,  9,  5,  6},
    // Sensor 11
    { -1, -1, 15, 10,  6, -1},
    // Sensor 12
    { 13, 16, -1, -1,  7,  8},
    // Sensor 13
    { 14, 17, 16, 12,  8,  9},
    // Sensor 14
    { 15, 18, 17, 13,  9, 10},
    // Sensor 15
    { -1, -1, 18, 14, 10, 11},
    // Sensor 16
    { 17, -1, -1, -1, 12, 13},
    // Sensor 17
    { 18, -1, -1, 16, 13, 14},
    // Sensor 18
    { -1, -1, -1, 17, 14, 15}
};

} // namespace UCF

#endif // HEX_GRID_H

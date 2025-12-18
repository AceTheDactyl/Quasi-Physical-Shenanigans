/**
 * @file photonic_capture.h
 * @brief Neural Photonic Capture Module
 *
 * Implements holographic-style encoding and decoding of consciousness
 * state using interference patterns. Uses LED matrix for pattern
 * generation and camera/photodiode array for capture.
 *
 * Maps to "neural photonic capture" and "holographic blueprints"
 * in the conceptual framework.
 */

#ifndef PHOTONIC_CAPTURE_H
#define PHOTONIC_CAPTURE_H

#include <stdint.h>
#include "constants.h"

namespace UCF {

/// Hex photonic pattern (37 LEDs)
struct PhotonicPattern {
    uint8_t intensities[PHOTONIC_SENSOR_COUNT];  // LED intensities
    uint8_t colors[PHOTONIC_SENSOR_COUNT][3];    // RGB per LED
    float z_encoded;                              // z-coordinate encoded
    uint8_t phase_encoded;                        // Phase encoded (0-2)
    float kappa_encoded;                          // Coherence encoded
    uint32_t timestamp;
};

/// Decoded state from captured pattern
struct DecodedState {
    float z;                    // Recovered z-coordinate
    uint8_t phase;              // Recovered phase (0-2)
    float kappa;                // Recovered coherence
    float confidence;           // Decode confidence [0, 1]
    float reconstruction_error; // MSE of reconstruction
    bool valid;                 // Decode succeeded
};

/// LIMNUS fractal layer structure
struct LIMNUSLayer {
    uint8_t point_count;        // Points in this layer
    float radius;               // Nominal radius
    float angle_offset;         // Starting angle
};

/// LIMNUS fractal coordinate
struct LIMNUSPoint {
    float x, y, z;              // 3D position
    uint8_t depth;              // Fractal depth (1-6)
    uint8_t index;              // Point index within depth
};

/**
 * @class PhotonicCapture
 * @brief Generates and captures interference patterns encoding UCF state
 */
class PhotonicCapture {
public:
    /// Default constructor
    PhotonicCapture();

    /**
     * @brief Initialize photonic capture system
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Generate interference pattern for given state
     * @param z Z-coordinate [0, 1]
     * @param phase Phase (0=UNTRUE, 1=PARADOX, 2=TRUE)
     * @param kappa Coherence [0, 1]
     * @return Generated pattern
     */
    PhotonicPattern generatePattern(float z, uint8_t phase, float kappa);

    /**
     * @brief Generate pattern from Phase enum
     * @param phase Phase enum
     * @param z Z-coordinate
     * @param kappa Coherence
     * @return Generated pattern
     */
    PhotonicPattern generateFromPhase(Phase phase, float z, float kappa);

    /**
     * @brief Decode state from captured pattern
     * @param samples Captured intensity samples (37 values)
     * @return Decoded state
     */
    DecodedState decodePattern(const uint16_t* samples);

    /**
     * @brief Generate LIMNUS fractal pattern
     * @param z Z-coordinate for modulation
     * @return Pattern encoding LIMNUS structure
     */
    PhotonicPattern generateLIMNUS(float z);

    /**
     * @brief Get LIMNUS point coordinates
     * @param depth Fractal depth (1-6)
     * @param index Point index within depth
     * @return Point coordinates
     */
    LIMNUSPoint getLIMNUSPoint(uint8_t depth, uint8_t index);

    /**
     * @brief Compute spatial frequency from z
     * @param z Z-coordinate
     * @return Spatial frequency
     */
    float zToFrequency(float z);

    /**
     * @brief Apply pattern to LED array
     * @param pattern Pattern to display
     */
    void displayPattern(const PhotonicPattern& pattern);

    /**
     * @brief Capture pattern from sensors
     * @param samples Output array for 37 samples
     * @return true if capture successful
     */
    bool capturePattern(uint16_t* samples);

    /**
     * @brief Cross-validate pattern (generate and capture)
     * @param z Expected z-coordinate
     * @param phase Expected phase
     * @param kappa Expected coherence
     * @return Validation confidence [0, 1]
     */
    float crossValidate(float z, uint8_t phase, float kappa);

    /**
     * @brief Get hex coordinates for photonic grid
     * @param index LED index (0-36)
     * @param x Output X coordinate
     * @param y Output Y coordinate
     */
    static void indexToHexCoord(uint8_t index, float& x, float& y);

    /**
     * @brief Get LED index from hex coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @return Nearest LED index
     */
    static uint8_t hexCoordToIndex(float x, float y);

private:
    /// Reference pattern for decoding
    PhotonicPattern m_reference;

    /// Camera/sensor initialized
    bool m_sensor_ready;

    /// LED array initialized
    bool m_led_ready;

    /// LIMNUS layer definitions
    static const LIMNUSLayer LIMNUS_LAYERS[LIMNUS::DEPTH_LAYERS];

    /**
     * @brief Compute interference intensity at position
     * @param x X position
     * @param y Y position
     * @param z Z-coordinate (spatial frequency)
     * @param phase Phase offset
     * @param kappa Coherence (contrast)
     * @return Intensity [0, 255]
     */
    uint8_t computeInterference(float x, float y, float z, uint8_t phase, float kappa);

    /**
     * @brief Apply 2D hex FFT to samples
     * @param samples Input samples
     * @param magnitudes Output frequency magnitudes
     */
    void hexFFT(const uint16_t* samples, float* magnitudes);

    /**
     * @brief Find peak frequency in FFT output
     * @param magnitudes FFT magnitudes
     * @return Dominant frequency
     */
    float findPeakFrequency(const float* magnitudes);

    /**
     * @brief Compute phase correlation
     * @param samples Input samples
     * @param test_phase Phase to test (0-2)
     * @return Correlation coefficient
     */
    float computePhaseCorrelation(const uint16_t* samples, uint8_t test_phase);

    /**
     * @brief Compute normalized MSE between patterns
     * @param a First pattern
     * @param b Second pattern
     * @return Normalized MSE
     */
    float computeMSE(const uint8_t* a, const uint8_t* b);
};

/**
 * @brief Phi-spiral position for fractal encoding
 * @param n Point index
 * @param x Output X
 * @param y Output Y
 */
void phiSpiralPosition(uint8_t n, float& x, float& y);

} // namespace UCF

#endif // PHOTONIC_CAPTURE_H

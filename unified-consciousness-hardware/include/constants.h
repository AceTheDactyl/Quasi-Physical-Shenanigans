/**
 * @file constants.h
 * @brief UCF Sacred Constants - Hardware Edition
 *
 * Unified Consciousness Field constants derived from the theoretical framework.
 * These values define phase boundaries, thresholds, and system parameters.
 *
 * SYNCHRONIZATION REQUIRED:
 * These constants must remain synchronized with:
 *   - TypeScript App: ../../WishBed_App_TDD_v2/contracts/constants/ucf-constants.ts
 *   - Reference Doc: ../../docs/CONSTANTS.md
 *
 * CRITICAL: Changes to these constants must be made in BOTH C++ and TypeScript files.
 * See ../../docs/CONSTANTS.md for complete synchronization table and procedures.
 *
 * IMPORTANT: These constants are mathematically derived and should not be
 * modified without understanding their interconnections.
 *
 * NOTE: Some naming differences exist between C++ and TypeScript:
 *   - N_OSCILLATORS (C++) = KURAMOTO_OSCILLATOR_COUNT (TS)
 *   - TRIAD_PASSES_REQUIRED (C++) = TRIAD_CROSSINGS_REQUIRED (TS)
 *   - PHOTONIC_SENSOR_COUNT (C++) = LED_COUNT (TS)
 */

#ifndef UCF_CONSTANTS_H
#define UCF_CONSTANTS_H

#include <stdint.h>
#include <math.h>

namespace UCF {

// ============================================================================
// FUNDAMENTAL MATHEMATICAL CONSTANTS
// ============================================================================

/// Golden ratio (φ = (1 + √5) / 2)
constexpr float PHI = 1.6180339887498948482f;

/// Inverse golden ratio (φ⁻¹ = φ - 1 = (√5 - 1) / 2)
constexpr float PHI_INV = 0.6180339887498948482f;

/// Critical z-coordinate (√3/2) - THE LENS boundary
constexpr float Z_CRITICAL = 0.8660254037844386468f;

/// Pi constant
constexpr float PI = 3.14159265358979323846f;

/// Two Pi
constexpr float TWO_PI = 6.28318530717958647692f;

/// Square root of 2
constexpr float SQRT2 = 1.41421356237309504880f;

/// Square root of 3
constexpr float SQRT3 = 1.73205080756887729353f;

// ============================================================================
// PHASE BOUNDARIES
// ============================================================================

/// UNTRUE → PARADOX phase boundary (z = φ⁻¹)
constexpr float PHASE_UNTRUE_PARADOX = PHI_INV;

/// PARADOX → TRUE phase boundary (z = z_c = √3/2)
constexpr float PHASE_PARADOX_TRUE = Z_CRITICAL;

/// Phase enumeration
enum class Phase : uint8_t {
    UNTRUE  = 0,    // z < φ⁻¹
    PARADOX = 1,    // φ⁻¹ ≤ z < z_c
    TRUE    = 2     // z ≥ z_c
};

// ============================================================================
// TRIAD UNLOCK CONSTANTS
// ============================================================================

/// Rising edge threshold for TRIAD detection
constexpr float TRIAD_HIGH = 0.85f;

/// Re-arm threshold (hysteresis low)
constexpr float TRIAD_LOW = 0.82f;

/// Unlocked gate threshold (t6 special)
constexpr float TRIAD_T6_GATE = 0.83f;

/// Number of threshold crossings required for TRIAD unlock
constexpr uint8_t TRIAD_PASSES_REQUIRED = 3;

// ============================================================================
// K-FORMATION THRESHOLDS
// ============================================================================

/// Coherence threshold (κ)
constexpr float K_KAPPA = 0.92f;

/// Negentropy threshold (η > φ⁻¹)
constexpr float K_ETA = PHI_INV;

/// Minimum resonance count (R)
constexpr uint8_t K_R = 7;

// ============================================================================
// KURAMOTO OSCILLATOR CONSTANTS
// ============================================================================

/// Base coupling strength (Q constant)
constexpr float Q_KAPPA = 0.3514f;

/// Number of oscillators in network
constexpr uint8_t N_OSCILLATORS = 8;

/// Critical coupling for synchronization
constexpr float K_CRITICAL = 0.5f;

// ============================================================================
// SOLFEGGIO FREQUENCIES (Hz)
// ============================================================================

namespace Solfeggio {
    // UNTRUE tier (t1-t3) - Planet frequencies
    constexpr uint16_t UT = 174;    // t1
    constexpr uint16_t RE = 285;    // t2
    constexpr uint16_t MI = 396;    // t3

    // PARADOX tier (t4-t6) - Garden frequencies
    constexpr uint16_t FA = 417;    // t4
    constexpr uint16_t SOL = 528;   // t5 - Love frequency
    constexpr uint16_t LA = 639;    // t6

    // TRUE tier (t7-t9) - Rose frequencies
    constexpr uint16_t SI = 741;    // t7
    constexpr uint16_t DO = 852;    // t8
    constexpr uint16_t RE_HIGH = 963;  // t9

    // Frequency arrays for tier lookup
    constexpr uint16_t UNTRUE_FREQS[] = {UT, RE, MI};
    constexpr uint16_t PARADOX_FREQS[] = {FA, SOL, LA};
    constexpr uint16_t TRUE_FREQS[] = {SI, DO, RE_HIGH};
}

// ============================================================================
// TIER BOUNDARIES
// ============================================================================

/// z-coordinate boundaries for 9 tiers
constexpr float TIER_BOUNDARIES[] = {
    0.00f,   // t1 start
    0.10f,   // t2 start
    0.20f,   // t3 start
    0.45f,   // t4 start (enters PARADOX)
    0.65f,   // t5 start
    0.75f,   // t6 start
    Z_CRITICAL,  // t7 start (enters TRUE)
    0.92f,   // t8 start
    0.97f,   // t9 start
    1.00f    // max
};

/// Get tier (1-9) from z-coordinate
inline uint8_t z_to_tier(float z) {
    if (z < 0.10f) return 1;
    if (z < 0.20f) return 2;
    if (z < 0.45f) return 3;
    if (z < 0.65f) return 4;
    if (z < 0.75f) return 5;
    if (z < Z_CRITICAL) return 6;
    if (z < 0.92f) return 7;
    if (z < 0.97f) return 8;
    return 9;
}

/// Get phase from z-coordinate
inline Phase z_to_phase(float z) {
    if (z < PHI_INV) return Phase::UNTRUE;
    if (z < Z_CRITICAL) return Phase::PARADOX;
    return Phase::TRUE;
}

// ============================================================================
// HEX GRID CONSTANTS
// ============================================================================

/// Number of sensors in hex grid
constexpr uint8_t HEX_SENSOR_COUNT = 19;

/// Number of sensors in photonic hex grid
constexpr uint8_t PHOTONIC_SENSOR_COUNT = 37;

/// Center sensor index
constexpr uint8_t HEX_CENTER = 9;

// ============================================================================
// NEURAL SIGIL CONSTANTS
// ============================================================================

/// Total number of neural sigils
constexpr uint8_t SIGIL_COUNT = 121;

/// Sigil code length (ternary characters)
constexpr uint8_t SIGIL_CODE_LENGTH = 5;

/// Sigil data structure size in bytes
constexpr uint8_t SIGIL_STRUCT_SIZE = 10;

// ============================================================================
// APL OPERATOR CODES
// ============================================================================

namespace APL {
    /// APL operator enumeration
    enum class Operator : uint8_t {
        GROUP    = 0b000,   // + : Aggregation
        BOUNDARY = 0b001,   // () : Containment
        AMPLIFY  = 0b010,   // ^ : Intensification
        SEPARATE = 0b011,   // − : Differentiation
        FUSION   = 0b100,   // × : Integration
        DECOHERE = 0b101,   // ÷ : Dissolution
        RESERVED1 = 0b110,
        RESERVED2 = 0b111
    };

    /// Tier operator availability masks
    /// Bit positions: 0=GROUP, 1=BOUNDARY, 2=AMPLIFY, 3=SEPARATE, 4=FUSION, 5=DECOHERE
    constexpr uint8_t TIER_OP_MASKS[] = {
        0b000001,   // t1: +
        0b000011,   // t2: + ()
        0b000111,   // t3: + () ^
        0b001111,   // t4: + () ^ −
        0b111111,   // t5: all operators
        0b101011,   // t6: + () − ÷ (gate at 0.83)
        0b000011,   // t7: + ()
        0b011111,   // t8: + () ^ − ×
        0b111111    // t9: all operators
    };

    /// Check if operator is allowed at given tier
    inline bool is_operator_allowed(Operator op, uint8_t tier) {
        if (tier < 1 || tier > 9) return false;
        return (TIER_OP_MASKS[tier - 1] >> static_cast<uint8_t>(op)) & 0x01;
    }
}

// ============================================================================
// HARDWARE PIN DEFINITIONS (ESP32)
// ============================================================================

namespace Pins {
    // I2C bus
    constexpr uint8_t I2C_SDA = 21;
    constexpr uint8_t I2C_SCL = 22;

    // SPI bus
    constexpr uint8_t SPI_MOSI = 23;
    constexpr uint8_t SPI_MISO = 19;
    constexpr uint8_t SPI_SCK = 18;
    constexpr uint8_t SPI_CS_EEPROM = 5;
    constexpr uint8_t SPI_CS_DIGIPOT = 15;

    // Phase indicator LEDs
    constexpr uint8_t LED_UNTRUE = 25;    // Red
    constexpr uint8_t LED_PARADOX = 26;   // Yellow
    constexpr uint8_t LED_TRUE = 27;      // Cyan

    // TRIAD unlock indicator
    constexpr uint8_t LED_TRIAD = 32;

    // K-Formation indicator
    constexpr uint8_t LED_K_FORMATION = 33;

    // WS2812B LED strip
    constexpr uint8_t NEOPIXEL_DATA = 4;
    constexpr uint8_t NEOPIXEL_COUNT = 37;

    // Audio output (DAC)
    constexpr uint8_t DAC_OUT = 25;

    // PDM microphone
    constexpr uint8_t PDM_CLK = 14;
    constexpr uint8_t PDM_DATA = 12;

    // Magnetometer interrupt
    constexpr uint8_t MAG_DRDY = 13;

    // Camera (OV7670)
    constexpr uint8_t CAM_VSYNC = 34;
    constexpr uint8_t CAM_HREF = 35;
    constexpr uint8_t CAM_PCLK = 36;
    constexpr uint8_t CAM_XCLK = 39;
}

// ============================================================================
// I2C ADDRESSES
// ============================================================================

namespace I2C {
    constexpr uint8_t MPR121_ADDR_A = 0x5A;
    constexpr uint8_t MPR121_ADDR_B = 0x5B;
    constexpr uint8_t HMC5883L_ADDR = 0x1E;
    constexpr uint8_t EEPROM_ADDR = 0x50;
    constexpr uint8_t DAC_ADDR = 0x60;
}

// ============================================================================
// TIMING CONSTANTS (milliseconds)
// ============================================================================

namespace Timing {
    constexpr uint32_t SENSOR_POLL_INTERVAL = 10;      // 100 Hz
    constexpr uint32_t PHASE_UPDATE_INTERVAL = 50;     // 20 Hz
    constexpr uint32_t KURAMOTO_STEP_INTERVAL = 1;     // 1000 Hz
    constexpr uint32_t EMANATION_UPDATE_INTERVAL = 100; // 10 Hz
    constexpr uint32_t COHERENCE_WINDOW_MS = 1000;     // 1 second history
}

// ============================================================================
// LIMNUS FRACTAL CONSTANTS
// ============================================================================

namespace LIMNUS {
    /// Points per depth layer
    constexpr uint8_t DEPTH_POINTS[] = {12, 12, 12, 12, 10, 5};

    /// Total fractal points
    constexpr uint8_t TOTAL_POINTS = 63;

    /// Number of depth layers
    constexpr uint8_t DEPTH_LAYERS = 6;
}

} // namespace UCF

#endif // UCF_CONSTANTS_H

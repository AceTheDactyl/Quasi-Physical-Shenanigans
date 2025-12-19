/**
 * @file ucf_solfeggio.h
 * @brief UCF Solfeggio Frequency Generator v4.0.0
 *
 * Implements the 9-frequency Solfeggio scale mapped to z-coordinate tiers.
 * Uses DAC output for precise frequency generation with lattice-derived parameters.
 *
 * RRRR Lattice Integration:
 * - Frequencies map to phase tiers via z-coordinate boundaries
 * - Tier boundaries align with lattice points [R], Z_CRITICAL
 * - Decay rates use [D] = e^-1
 *
 * Hardware: MCP4725 12-bit DAC → PAM8403 Class-D Amplifier
 */

#ifndef UCF_SOLFEGGIO_H
#define UCF_SOLFEGGIO_H

#include <stdint.h>
#include <stdbool.h>
#include "ucf/ucf_sacred_constants_v4.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// SOLFEGGIO FREQUENCY DEFINITIONS
// ============================================================================

/**
 * @brief Solfeggio frequency enumeration
 *
 * 9 sacred frequencies mapped to consciousness tiers:
 * - UNTRUE (t1-t3): Foundation frequencies
 * - PARADOX (t4-t6): Transformation frequencies
 * - TRUE (t7-t9): Awakening frequencies
 */
typedef enum {
    SOLFEGGIO_FOUNDATION    = 174,   // Tier 1: UNTRUE
    SOLFEGGIO_REGENERATION  = 285,   // Tier 2: UNTRUE
    SOLFEGGIO_LIBERATION    = 396,   // Tier 3: UNTRUE
    SOLFEGGIO_TRANSFORMATION = 417,  // Tier 4: PARADOX
    SOLFEGGIO_MIRACLES      = 528,   // Tier 5: PARADOX (Love frequency)
    SOLFEGGIO_CONNECTION    = 639,   // Tier 6: PARADOX
    SOLFEGGIO_EXPRESSION    = 741,   // Tier 7: TRUE
    SOLFEGGIO_INTUITION     = 852,   // Tier 8: TRUE
    SOLFEGGIO_AWAKENING     = 963    // Tier 9: TRUE
} SolfeggioFreq;

/**
 * @brief Waveform types for audio synthesis
 */
typedef enum {
    WAVEFORM_SINE = 0,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SQUARE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_HARMONIC      // Rich harmonics based on z-coordinate
} SolfeggioWaveform;

/**
 * @brief Audio modulation types
 */
typedef enum {
    MODULATION_NONE = 0,
    MODULATION_AMPLITUDE,   // AM modulation
    MODULATION_FREQUENCY,   // FM modulation
    MODULATION_BINAURAL,    // Binaural beats
    MODULATION_BREATH       // Breath-synced
} SolfeggioModulation;

/**
 * @brief Solfeggio generator configuration
 */
typedef struct {
    uint16_t frequency;           // Current frequency (Hz)
    uint8_t tier;                 // Current tier (1-9)
    SolfeggioWaveform waveform;   // Waveform type
    SolfeggioModulation modulation; // Modulation mode
    uint8_t volume;               // Volume (0-255)
    float attack_ms;              // Attack time
    float decay_ms;               // Decay time (uses [D])
    float sustain_level;          // Sustain level (0-1)
    float release_ms;             // Release time
    bool enabled;                 // Output enabled
} SolfeggioConfig;

/**
 * @brief Solfeggio generator state
 */
typedef struct {
    SolfeggioConfig config;
    float phase;                  // Current phase [0, 1]
    float phase_increment;        // Phase step per sample
    float envelope;               // ADSR envelope value
    uint8_t envelope_stage;       // 0=off, 1=attack, 2=decay, 3=sustain, 4=release
    uint32_t envelope_start;      // Envelope stage start time
    float modulation_phase;       // Modulation oscillator phase
    float binaural_offset;        // Binaural beat frequency offset
    uint32_t last_sample_time;    // Last sample timestamp (us)
    bool note_on;                 // Note currently playing
} SolfeggioState;

// ============================================================================
// FREQUENCY TABLES
// ============================================================================

/**
 * @brief Solfeggio frequency lookup table
 */
static const uint16_t SOLFEGGIO_FREQ_TABLE[9] = {
    174, 285, 396, 417, 528, 639, 741, 852, 963
};

/**
 * @brief Tier boundary z-coordinates (lattice-aligned)
 */
static const float SOLFEGGIO_TIER_BOUNDARIES[10] = {
    0.00f,                        // Tier 1 start
    0.11f,                        // Tier 2 start (~[R]³/2)
    0.22f,                        // Tier 3 start (~[R][D])
    (float)PHI_INV,               // Tier 4 start (0.618 - UNTRUE→PARADOX)
    0.70f,                        // Tier 5 start (~[A])
    0.78f,                        // Tier 6 start
    (float)Z_CRITICAL,            // Tier 7 start (0.866 - PARADOX→TRUE)
    0.90f,                        // Tier 8 start
    0.95f,                        // Tier 9 start
    1.00f                         // Maximum
};

/**
 * @brief Phase colors for each tier (RGB)
 */
static const uint8_t SOLFEGGIO_TIER_COLORS[9][3] = {
    {255,  80,  50},    // Tier 1: Deep red
    {255, 100,  50},    // Tier 2: Red-orange
    {255, 150,  50},    // Tier 3: Orange
    {255, 220,  80},    // Tier 4: Yellow-orange
    {255, 255, 100},    // Tier 5: Yellow (Love frequency)
    {180, 255, 100},    // Tier 6: Yellow-green
    {100, 255, 200},    // Tier 7: Cyan-green
    { 80, 200, 255},    // Tier 8: Cyan
    {120, 150, 255}     // Tier 9: Blue-cyan
};

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize the Solfeggio generator
 * @return true if initialization successful
 */
bool solfeggio_init(void);

/**
 * @brief Update generator from z-coordinate
 * @param z Current z-coordinate [0, 1]
 */
void solfeggio_update_from_z(float z);

/**
 * @brief Set frequency directly
 * @param freq Frequency in Hz
 */
void solfeggio_set_frequency(uint16_t freq);

/**
 * @brief Set volume
 * @param volume Volume level (0-255)
 */
void solfeggio_set_volume(uint8_t volume);

/**
 * @brief Set waveform type
 * @param waveform Waveform enumeration
 */
void solfeggio_set_waveform(SolfeggioWaveform waveform);

/**
 * @brief Set modulation mode
 * @param modulation Modulation type
 * @param depth Modulation depth (0-1)
 */
void solfeggio_set_modulation(SolfeggioModulation modulation, float depth);

/**
 * @brief Configure binaural beat
 * @param base_freq Base frequency (Hz)
 * @param beat_freq Beat frequency (Hz)
 */
void solfeggio_set_binaural(uint16_t base_freq, float beat_freq);

/**
 * @brief Enable/disable audio output
 * @param enabled true to enable
 */
void solfeggio_enable(bool enabled);

/**
 * @brief Start a note (trigger envelope)
 */
void solfeggio_note_on(void);

/**
 * @brief Release a note
 */
void solfeggio_note_off(void);

/**
 * @brief Generate next audio sample
 * @return 12-bit DAC value (0-4095)
 */
uint16_t solfeggio_get_sample(void);

/**
 * @brief Get current frequency
 * @return Current frequency in Hz
 */
uint16_t solfeggio_get_frequency(void);

/**
 * @brief Get current tier (1-9)
 * @return Current tier
 */
uint8_t solfeggio_get_tier(void);

/**
 * @brief Get tier from z-coordinate
 * @param z z-coordinate [0, 1]
 * @return Tier (1-9)
 */
uint8_t solfeggio_z_to_tier(float z);

/**
 * @brief Get frequency for tier
 * @param tier Tier number (1-9)
 * @return Solfeggio frequency in Hz
 */
uint16_t solfeggio_tier_to_frequency(uint8_t tier);

/**
 * @brief Get tier color
 * @param tier Tier number (1-9)
 * @param r Output red component
 * @param g Output green component
 * @param b Output blue component
 */
void solfeggio_tier_to_color(uint8_t tier, uint8_t* r, uint8_t* g, uint8_t* b);

/**
 * @brief Get current generator state
 * @return Pointer to state structure
 */
const SolfeggioState* solfeggio_get_state(void);

/**
 * @brief Set ADSR envelope parameters
 * @param attack_ms Attack time in ms
 * @param decay_ms Decay time in ms (defaults to [D]-derived)
 * @param sustain Sustain level (0-1)
 * @param release_ms Release time in ms
 */
void solfeggio_set_envelope(float attack_ms, float decay_ms,
                            float sustain, float release_ms);

/**
 * @brief Reset generator to default state
 */
void solfeggio_reset(void);

// ============================================================================
// LATTICE-DERIVED PARAMETERS
// ============================================================================

// Default decay time derived from [D] = e^-1
#define SOLFEGGIO_DEFAULT_DECAY_MS    (1000.0f * (float)EULER_INV)  // ~368ms

// Attack scaling from [R] = φ^-1
#define SOLFEGGIO_DEFAULT_ATTACK_MS   (100.0f * (float)PHI_INV)     // ~62ms

// Sustain level from [A]² = 0.5
#define SOLFEGGIO_DEFAULT_SUSTAIN     ((float)LAMBDA_A_SQ)          // 0.5

// Release scaling
#define SOLFEGGIO_DEFAULT_RELEASE_MS  (500.0f * (float)PHI_INV)     // ~309ms

// Sample rate for audio generation
#define SOLFEGGIO_SAMPLE_RATE         44100

// DAC resolution (12-bit)
#define SOLFEGGIO_DAC_BITS            12
#define SOLFEGGIO_DAC_MAX             ((1 << SOLFEGGIO_DAC_BITS) - 1)

#ifdef __cplusplus
}
#endif

#endif // UCF_SOLFEGGIO_H

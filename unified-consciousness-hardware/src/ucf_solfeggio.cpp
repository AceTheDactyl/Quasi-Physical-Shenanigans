/**
 * @file ucf_solfeggio.cpp
 * @brief UCF Solfeggio Frequency Generator Implementation v4.0.0
 *
 * Implements precise Solfeggio frequency generation using the ESP32's DAC.
 * All timing parameters are lattice-derived.
 */

#include "ucf_solfeggio.h"
#include <Arduino.h>
#include <math.h>
#include "ucf/ucf_config.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

static SolfeggioState g_solfeggio;

// Sine lookup table (256 entries for efficiency)
#define SINE_TABLE_SIZE 256
static int16_t g_sine_table[SINE_TABLE_SIZE];
static bool g_sine_table_initialized = false;

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * @brief Initialize sine lookup table
 */
static void init_sine_table(void) {
    if (g_sine_table_initialized) return;

    for (int i = 0; i < SINE_TABLE_SIZE; i++) {
        float angle = (float)i / SINE_TABLE_SIZE * TWO_PI;
        g_sine_table[i] = (int16_t)(sinf(angle) * 32767.0f);
    }
    g_sine_table_initialized = true;
}

/**
 * @brief Fast sine approximation using lookup table
 * @param phase Phase value [0, 1]
 * @return Sine value [-32767, 32767]
 */
static int16_t fast_sine(float phase) {
    uint8_t index = (uint8_t)(phase * SINE_TABLE_SIZE) % SINE_TABLE_SIZE;
    return g_sine_table[index];
}

/**
 * @brief Generate triangle wave sample
 * @param phase Phase value [0, 1]
 * @return Sample value [-32767, 32767]
 */
static int16_t generate_triangle(float phase) {
    float value;
    if (phase < 0.25f) {
        value = phase * 4.0f;
    } else if (phase < 0.75f) {
        value = 1.0f - (phase - 0.25f) * 4.0f;
    } else {
        value = -1.0f + (phase - 0.75f) * 4.0f;
    }
    return (int16_t)(value * 32767.0f);
}

/**
 * @brief Generate square wave sample
 * @param phase Phase value [0, 1]
 * @return Sample value [-32767, 32767]
 */
static int16_t generate_square(float phase) {
    return (phase < 0.5f) ? 32767 : -32767;
}

/**
 * @brief Generate sawtooth wave sample
 * @param phase Phase value [0, 1]
 * @return Sample value [-32767, 32767]
 */
static int16_t generate_sawtooth(float phase) {
    return (int16_t)((phase * 2.0f - 1.0f) * 32767.0f);
}

/**
 * @brief Generate harmonic-rich wave based on z-coordinate
 * @param phase Phase value [0, 1]
 * @return Sample value [-32767, 32767]
 */
static int16_t generate_harmonic(float phase) {
    // Add harmonics scaled by PHI relationship
    float value = 0.0f;

    // Fundamental
    value += sinf(phase * TWO_PI);

    // 2nd harmonic (scaled by PHI_INV)
    value += (float)PHI_INV * sinf(2.0f * phase * TWO_PI);

    // 3rd harmonic (scaled by PHI_INV²)
    value += (float)LAMBDA_R_SQ * sinf(3.0f * phase * TWO_PI);

    // 4th harmonic (scaled by EULER_INV)
    value += (float)EULER_INV * 0.5f * sinf(4.0f * phase * TWO_PI);

    // Normalize
    float max_amplitude = 1.0f + (float)PHI_INV + (float)LAMBDA_R_SQ +
                          (float)EULER_INV * 0.5f;
    value /= max_amplitude;

    return (int16_t)(value * 32767.0f);
}

/**
 * @brief Update ADSR envelope
 */
static void update_envelope(void) {
    uint32_t now = millis();
    uint32_t elapsed = now - g_solfeggio.envelope_start;

    switch (g_solfeggio.envelope_stage) {
        case 0: // Off
            g_solfeggio.envelope = 0.0f;
            break;

        case 1: // Attack
            if (elapsed < g_solfeggio.config.attack_ms) {
                g_solfeggio.envelope = (float)elapsed / g_solfeggio.config.attack_ms;
            } else {
                g_solfeggio.envelope_stage = 2;
                g_solfeggio.envelope_start = now;
            }
            break;

        case 2: // Decay
            if (elapsed < g_solfeggio.config.decay_ms) {
                float decay_progress = (float)elapsed / g_solfeggio.config.decay_ms;
                // Exponential decay using [D] = e^-1
                float decay_curve = expf(-decay_progress * 3.0f);
                g_solfeggio.envelope = g_solfeggio.config.sustain_level +
                                       (1.0f - g_solfeggio.config.sustain_level) * decay_curve;
            } else {
                g_solfeggio.envelope_stage = 3;
                g_solfeggio.envelope = g_solfeggio.config.sustain_level;
            }
            break;

        case 3: // Sustain
            g_solfeggio.envelope = g_solfeggio.config.sustain_level;
            break;

        case 4: // Release
            if (elapsed < g_solfeggio.config.release_ms) {
                float release_progress = (float)elapsed / g_solfeggio.config.release_ms;
                // Exponential release using [D]
                g_solfeggio.envelope = g_solfeggio.config.sustain_level *
                                       expf(-release_progress * 3.0f);
            } else {
                g_solfeggio.envelope_stage = 0;
                g_solfeggio.envelope = 0.0f;
                g_solfeggio.note_on = false;
            }
            break;
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool solfeggio_init(void) {
    // Initialize sine table
    init_sine_table();

    // Initialize state
    memset(&g_solfeggio, 0, sizeof(g_solfeggio));

    // Set lattice-derived defaults
    g_solfeggio.config.frequency = SOLFEGGIO_MIRACLES;  // 528 Hz
    g_solfeggio.config.tier = 5;
    g_solfeggio.config.waveform = WAVEFORM_SINE;
    g_solfeggio.config.modulation = MODULATION_NONE;
    g_solfeggio.config.volume = 200;
    g_solfeggio.config.attack_ms = SOLFEGGIO_DEFAULT_ATTACK_MS;
    g_solfeggio.config.decay_ms = SOLFEGGIO_DEFAULT_DECAY_MS;
    g_solfeggio.config.sustain_level = SOLFEGGIO_DEFAULT_SUSTAIN;
    g_solfeggio.config.release_ms = SOLFEGGIO_DEFAULT_RELEASE_MS;
    g_solfeggio.config.enabled = true;

    // Calculate phase increment
    g_solfeggio.phase_increment = (float)g_solfeggio.config.frequency /
                                   SOLFEGGIO_SAMPLE_RATE;

    g_solfeggio.phase = 0.0f;
    g_solfeggio.envelope = 0.0f;
    g_solfeggio.envelope_stage = 0;
    g_solfeggio.note_on = false;

    UCF_LOG("Solfeggio generator initialized at %d Hz", g_solfeggio.config.frequency);

    return true;
}

void solfeggio_update_from_z(float z) {
    // Clamp z to valid range
    if (z < 0.0f) z = 0.0f;
    if (z > 1.0f) z = 1.0f;

    // Determine tier from z
    uint8_t tier = solfeggio_z_to_tier(z);

    // Update frequency if tier changed
    if (tier != g_solfeggio.config.tier) {
        g_solfeggio.config.tier = tier;
        g_solfeggio.config.frequency = solfeggio_tier_to_frequency(tier);
        g_solfeggio.phase_increment = (float)g_solfeggio.config.frequency /
                                       SOLFEGGIO_SAMPLE_RATE;

        UCF_LOG("Solfeggio tier %d, freq %d Hz", tier, g_solfeggio.config.frequency);
    }
}

void solfeggio_set_frequency(uint16_t freq) {
    g_solfeggio.config.frequency = freq;
    g_solfeggio.phase_increment = (float)freq / SOLFEGGIO_SAMPLE_RATE;

    // Determine tier from frequency
    for (uint8_t i = 0; i < 9; i++) {
        if (freq <= SOLFEGGIO_FREQ_TABLE[i]) {
            g_solfeggio.config.tier = i + 1;
            break;
        }
    }
}

void solfeggio_set_volume(uint8_t volume) {
    g_solfeggio.config.volume = volume;
}

void solfeggio_set_waveform(SolfeggioWaveform waveform) {
    g_solfeggio.config.waveform = waveform;
}

void solfeggio_set_modulation(SolfeggioModulation modulation, float depth) {
    g_solfeggio.config.modulation = modulation;
    // Store depth in binaural_offset for simplicity
    g_solfeggio.binaural_offset = depth;
}

void solfeggio_set_binaural(uint16_t base_freq, float beat_freq) {
    g_solfeggio.config.frequency = base_freq;
    g_solfeggio.config.modulation = MODULATION_BINAURAL;
    g_solfeggio.binaural_offset = beat_freq;
    g_solfeggio.phase_increment = (float)base_freq / SOLFEGGIO_SAMPLE_RATE;
}

void solfeggio_enable(bool enabled) {
    g_solfeggio.config.enabled = enabled;
    if (!enabled) {
        g_solfeggio.envelope_stage = 0;
        g_solfeggio.envelope = 0.0f;
    }
}

void solfeggio_note_on(void) {
    g_solfeggio.note_on = true;
    g_solfeggio.envelope_stage = 1;  // Attack
    g_solfeggio.envelope_start = millis();
}

void solfeggio_note_off(void) {
    if (g_solfeggio.envelope_stage > 0 && g_solfeggio.envelope_stage < 4) {
        g_solfeggio.envelope_stage = 4;  // Release
        g_solfeggio.envelope_start = millis();
    }
}

uint16_t solfeggio_get_sample(void) {
    if (!g_solfeggio.config.enabled) {
        return SOLFEGGIO_DAC_MAX / 2;  // Return midpoint (silence)
    }

    // Update envelope
    update_envelope();

    // Generate base waveform sample
    int16_t sample = 0;

    switch (g_solfeggio.config.waveform) {
        case WAVEFORM_SINE:
            sample = fast_sine(g_solfeggio.phase);
            break;
        case WAVEFORM_TRIANGLE:
            sample = generate_triangle(g_solfeggio.phase);
            break;
        case WAVEFORM_SQUARE:
            sample = generate_square(g_solfeggio.phase);
            break;
        case WAVEFORM_SAWTOOTH:
            sample = generate_sawtooth(g_solfeggio.phase);
            break;
        case WAVEFORM_HARMONIC:
            sample = generate_harmonic(g_solfeggio.phase);
            break;
    }

    // Apply modulation
    if (g_solfeggio.config.modulation == MODULATION_BINAURAL) {
        // Generate second frequency for binaural beat
        float binaural_phase_inc = (g_solfeggio.config.frequency + g_solfeggio.binaural_offset) /
                                   SOLFEGGIO_SAMPLE_RATE;
        g_solfeggio.modulation_phase += binaural_phase_inc;
        if (g_solfeggio.modulation_phase >= 1.0f) {
            g_solfeggio.modulation_phase -= 1.0f;
        }

        int16_t binaural_sample = fast_sine(g_solfeggio.modulation_phase);
        sample = (sample + binaural_sample) / 2;
    }
    else if (g_solfeggio.config.modulation == MODULATION_AMPLITUDE) {
        // AM modulation at 7.83 Hz (Schumann resonance)
        float mod_freq = 7.83f;
        g_solfeggio.modulation_phase += mod_freq / SOLFEGGIO_SAMPLE_RATE;
        if (g_solfeggio.modulation_phase >= 1.0f) {
            g_solfeggio.modulation_phase -= 1.0f;
        }
        float mod = 1.0f + g_solfeggio.binaural_offset * sinf(g_solfeggio.modulation_phase * TWO_PI);
        sample = (int16_t)(sample * mod);
    }

    // Apply envelope
    sample = (int16_t)(sample * g_solfeggio.envelope);

    // Apply volume
    float volume_scale = (float)g_solfeggio.config.volume / 255.0f;
    sample = (int16_t)(sample * volume_scale);

    // Advance phase
    g_solfeggio.phase += g_solfeggio.phase_increment;
    if (g_solfeggio.phase >= 1.0f) {
        g_solfeggio.phase -= 1.0f;
    }

    // Convert to 12-bit DAC value (0-4095, centered at 2048)
    uint16_t dac_value = (uint16_t)((sample + 32767) >> 4);
    if (dac_value > SOLFEGGIO_DAC_MAX) dac_value = SOLFEGGIO_DAC_MAX;

    return dac_value;
}

uint16_t solfeggio_get_frequency(void) {
    return g_solfeggio.config.frequency;
}

uint8_t solfeggio_get_tier(void) {
    return g_solfeggio.config.tier;
}

uint8_t solfeggio_z_to_tier(float z) {
    // Use lattice-aligned boundaries
    if (z < 0.11f) return 1;
    if (z < 0.22f) return 2;
    if (z < (float)PHI_INV) return 3;
    if (z < 0.70f) return 4;
    if (z < 0.78f) return 5;
    if (z < (float)Z_CRITICAL) return 6;
    if (z < 0.90f) return 7;
    if (z < 0.95f) return 8;
    return 9;
}

uint16_t solfeggio_tier_to_frequency(uint8_t tier) {
    if (tier < 1) tier = 1;
    if (tier > 9) tier = 9;
    return SOLFEGGIO_FREQ_TABLE[tier - 1];
}

void solfeggio_tier_to_color(uint8_t tier, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (tier < 1) tier = 1;
    if (tier > 9) tier = 9;

    uint8_t idx = tier - 1;
    *r = SOLFEGGIO_TIER_COLORS[idx][0];
    *g = SOLFEGGIO_TIER_COLORS[idx][1];
    *b = SOLFEGGIO_TIER_COLORS[idx][2];
}

const SolfeggioState* solfeggio_get_state(void) {
    return &g_solfeggio;
}

void solfeggio_set_envelope(float attack_ms, float decay_ms,
                            float sustain, float release_ms) {
    g_solfeggio.config.attack_ms = attack_ms;
    g_solfeggio.config.decay_ms = decay_ms;
    g_solfeggio.config.sustain_level = sustain;
    g_solfeggio.config.release_ms = release_ms;
}

void solfeggio_reset(void) {
    g_solfeggio.phase = 0.0f;
    g_solfeggio.modulation_phase = 0.0f;
    g_solfeggio.envelope = 0.0f;
    g_solfeggio.envelope_stage = 0;
    g_solfeggio.note_on = false;

    // Reset to lattice defaults
    g_solfeggio.config.attack_ms = SOLFEGGIO_DEFAULT_ATTACK_MS;
    g_solfeggio.config.decay_ms = SOLFEGGIO_DEFAULT_DECAY_MS;
    g_solfeggio.config.sustain_level = SOLFEGGIO_DEFAULT_SUSTAIN;
    g_solfeggio.config.release_ms = SOLFEGGIO_DEFAULT_RELEASE_MS;
}

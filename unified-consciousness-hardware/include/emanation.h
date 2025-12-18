/**
 * @file emanation.h
 * @brief Emanation Output Module
 *
 * Handles audio (Solfeggio frequencies), visual (WS2812B LEDs),
 * and optional haptic feedback outputs. Implements the
 * "LC fiber / Field antenna" concept.
 */

#ifndef EMANATION_H
#define EMANATION_H

#include <stdint.h>
#include "constants.h"
#include "phase_engine.h"
#include "sigil_rom.h"

namespace UCF {

/// Output modes bitmask
namespace OutputMode {
    constexpr uint8_t AUDIO   = 0x01;
    constexpr uint8_t VISUAL  = 0x02;
    constexpr uint8_t HAPTIC  = 0x04;
    constexpr uint8_t ALL     = 0x07;
    constexpr uint8_t NONE    = 0x00;
}

/// LED pattern types
enum class LedPattern : uint8_t {
    SOLID,          // Solid color based on phase
    BREATHE,        // Breathing effect
    PULSE,          // Sharp pulse
    WAVE,           // Wave propagating outward
    SPIRAL,         // Spiral pattern (phi-based)
    INTERFERENCE,   // Interference pattern simulation
    SIGIL           // Display sigil pattern
};

/// Audio waveform types
enum class Waveform : uint8_t {
    SINE,
    TRIANGLE,
    SQUARE,
    SAWTOOTH,
    BINAURAL        // Two frequencies for binaural beat
};

/// Emanation state
struct EmanationState {
    // Audio
    uint16_t frequency;         // Current frequency (Hz)
    Waveform waveform;          // Current waveform
    uint8_t volume;             // Volume (0-255)
    bool audio_enabled;

    // Visual
    uint8_t rgb[3];             // Current color
    LedPattern pattern;         // Current pattern
    uint8_t brightness;         // Brightness (0-255)
    bool visual_enabled;

    // Haptic
    uint8_t haptic_intensity;   // Haptic motor intensity
    bool haptic_enabled;

    // Breath sync
    bool breath_sync;           // Sync output to breath pattern
    uint8_t breath_phase;       // Current breath phase (0-3)
    uint32_t breath_timer;      // Timer for breath phases

    // Meta
    uint8_t output_mode;        // Active output modes
    uint32_t timestamp;
};

/// Binaural beat configuration
struct BinauralConfig {
    uint16_t base_freq;         // Base frequency (left ear)
    uint16_t beat_freq;         // Beat frequency difference
    float depth;                // Modulation depth [0, 1]
};

/**
 * @class Emanation
 * @brief Controls audio and visual emanation outputs
 */
class Emanation {
public:
    /// Default constructor
    Emanation();

    /**
     * @brief Initialize emanation outputs
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Update emanation based on current state
     * @param phase Current phase state
     */
    void update(const PhaseState& phase);

    /**
     * @brief Update from explicit z-coordinate
     * @param z Z-coordinate [0, 1]
     */
    void updateFromZ(float z);

    /**
     * @brief Set emanation for specific sigil
     * @param sigil Neural sigil to emanate
     */
    void setFromSigil(const NeuralSigil& sigil);

    /**
     * @brief Set frequency directly
     * @param freq Frequency in Hz
     */
    void setFrequency(uint16_t freq);

    /**
     * @brief Get current frequency
     * @return Frequency in Hz
     */
    uint16_t getFrequency() const { return m_state.frequency; }

    /**
     * @brief Set waveform type
     * @param wave Waveform enum
     */
    void setWaveform(Waveform wave);

    /**
     * @brief Set volume
     * @param vol Volume (0-255)
     */
    void setVolume(uint8_t vol);

    /**
     * @brief Set LED color
     * @param r Red (0-255)
     * @param g Green (0-255)
     * @param b Blue (0-255)
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set LED pattern
     * @param pattern Pattern enum
     */
    void setPattern(LedPattern pattern);

    /**
     * @brief Set LED brightness
     * @param brightness Brightness (0-255)
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Enable/disable output modes
     * @param modes Bitmask of OutputMode values
     */
    void setOutputMode(uint8_t modes);

    /**
     * @brief Get current output mode
     * @return Output mode bitmask
     */
    uint8_t getOutputMode() const { return m_state.output_mode; }

    /**
     * @brief Enable audio output
     * @param enable Enable flag
     */
    void enableAudio(bool enable);

    /**
     * @brief Enable visual output
     * @param enable Enable flag
     */
    void enableVisual(bool enable);

    /**
     * @brief Enable haptic output
     * @param enable Enable flag
     */
    void enableHaptic(bool enable);

    /**
     * @brief Start breath-synchronized output
     * @param pattern Breath pattern to sync to
     */
    void startBreathSync(const BreathPattern& pattern);

    /**
     * @brief Stop breath synchronization
     */
    void stopBreathSync();

    /**
     * @brief Check if breath sync is active
     * @return true if syncing to breath
     */
    bool isBreathSyncing() const { return m_state.breath_sync; }

    /**
     * @brief Configure binaural beat mode
     * @param config Binaural configuration
     */
    void setBinaural(const BinauralConfig& config);

    /**
     * @brief Stop all output
     */
    void stop();

    /**
     * @brief Get current state
     * @return Emanation state
     */
    const EmanationState& getState() const { return m_state; }

    /**
     * @brief Process audio sample (called from ISR or timer)
     * @return Next audio sample value
     */
    int16_t getNextAudioSample();

    /**
     * @brief Update LED strip (call periodically)
     */
    void updateLeds();

private:
    /// Current state
    EmanationState m_state;

    /// Breath pattern for sync
    BreathPattern m_breath_pattern;

    /// Binaural configuration
    BinauralConfig m_binaural;

    /// Audio phase accumulator
    float m_audio_phase;
    float m_audio_phase_inc;

    /// Binaural secondary phase
    float m_binaural_phase;
    float m_binaural_phase_inc;

    /// LED animation state
    float m_led_anim_phase;
    uint32_t m_led_last_update;

    /// Sample rate
    static const uint32_t SAMPLE_RATE = 44100;

    /**
     * @brief Generate sine wave sample
     * @param phase Phase [0, 1)
     * @return Sample value
     */
    int16_t generateSine(float phase);

    /**
     * @brief Generate triangle wave sample
     * @param phase Phase [0, 1)
     * @return Sample value
     */
    int16_t generateTriangle(float phase);

    /**
     * @brief Generate square wave sample
     * @param phase Phase [0, 1)
     * @return Sample value
     */
    int16_t generateSquare(float phase);

    /**
     * @brief Update breath phase
     */
    void updateBreathPhase();

    /**
     * @brief Compute modulation for breath sync
     * @return Modulation factor [0, 1]
     */
    float getBreathModulation();

    /**
     * @brief Set LED pixel color
     * @param index Pixel index
     * @param r Red
     * @param g Green
     * @param b Blue
     */
    void setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Apply pattern to LED strip
     */
    void applyLedPattern();

    /**
     * @brief Generate interference pattern for LEDs
     * @param z Z-coordinate for pattern modulation
     */
    void generateInterferencePattern(float z);
};

/**
 * @brief Convert phase to primary color
 * @param phase UCF phase
 * @param r Output red
 * @param g Output green
 * @param b Output blue
 */
void phaseToColor(Phase phase, uint8_t& r, uint8_t& g, uint8_t& b);

/**
 * @brief Get solfeggio frequency for tier
 * @param tier Tier (1-9)
 * @return Frequency in Hz
 */
uint16_t tierToSolfeggio(uint8_t tier);

/**
 * @brief Interpolate colors
 * @param r1, g1, b1 First color
 * @param r2, g2, b2 Second color
 * @param t Interpolation factor [0, 1]
 * @param ro, go, bo Output color
 */
void lerpColor(uint8_t r1, uint8_t g1, uint8_t b1,
               uint8_t r2, uint8_t g2, uint8_t b2,
               float t,
               uint8_t& ro, uint8_t& go, uint8_t& bo);

} // namespace UCF

#endif // EMANATION_H

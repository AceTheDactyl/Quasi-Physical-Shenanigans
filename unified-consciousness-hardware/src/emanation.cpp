/**
 * @file emanation.cpp
 * @brief Implementation of Emanation Output Module
 */

#include "emanation.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <math.h>
#include <string.h>

namespace UCF {

// NeoPixel strip instance
static Adafruit_NeoPixel strip(Pins::NEOPIXEL_COUNT, Pins::NEOPIXEL_DATA, NEO_GRB + NEO_KHZ800);

// Phase colors
static const uint8_t PHASE_COLORS[3][3] = {
    {255, 80, 50},     // UNTRUE: Red/Orange
    {220, 255, 80},    // PARADOX: Yellow/Green
    {80, 200, 255}     // TRUE: Cyan
};

Emanation::Emanation()
    : m_audio_phase(0.0f)
    , m_audio_phase_inc(0.0f)
    , m_binaural_phase(0.0f)
    , m_binaural_phase_inc(0.0f)
    , m_led_anim_phase(0.0f)
    , m_led_last_update(0)
{
    memset(&m_state, 0, sizeof(m_state));
    memset(&m_breath_pattern, 0, sizeof(m_breath_pattern));
    memset(&m_binaural, 0, sizeof(m_binaural));

    // Defaults
    m_state.frequency = Solfeggio::SOL;  // 528 Hz
    m_state.waveform = Waveform::SINE;
    m_state.volume = 128;
    m_state.brightness = 128;
    m_state.pattern = LedPattern::SOLID;
    m_state.output_mode = OutputMode::ALL;
    m_state.rgb[0] = 220;
    m_state.rgb[1] = 255;
    m_state.rgb[2] = 80;
}

bool Emanation::begin() {
    // Initialize NeoPixels
    strip.begin();
    strip.setBrightness(m_state.brightness);
    strip.show();

    // Initialize DAC for audio (ESP32 has built-in DAC)
    // DAC output on GPIO25
    pinMode(Pins::DAC_OUT, OUTPUT);

    return true;
}

void Emanation::update(const PhaseState& phase) {
    m_state.timestamp = millis();

    // Update frequency from tier
    m_state.frequency = tierToSolfeggio(phase.tier);

    // Update color from phase
    phaseToColor(phase.current, m_state.rgb[0], m_state.rgb[1], m_state.rgb[2]);

    // Calculate phase increment for audio
    m_audio_phase_inc = static_cast<float>(m_state.frequency) / SAMPLE_RATE;

    // Update breath phase if syncing
    if (m_state.breath_sync) {
        updateBreathPhase();
    }

    // Update LED display
    if (m_state.visual_enabled) {
        updateLeds();
    }
}

void Emanation::updateFromZ(float z) {
    uint8_t tier = z_to_tier(z);
    Phase phase = z_to_phase(z);

    m_state.frequency = tierToSolfeggio(tier);
    m_audio_phase_inc = static_cast<float>(m_state.frequency) / SAMPLE_RATE;

    phaseToColor(phase, m_state.rgb[0], m_state.rgb[1], m_state.rgb[2]);

    if (m_state.breath_sync) {
        updateBreathPhase();
    }

    if (m_state.visual_enabled) {
        updateLeds();
    }
}

void Emanation::setFromSigil(const NeuralSigil& sigil) {
    // Set frequency from sigil
    m_state.frequency = sigil.frequency;
    m_audio_phase_inc = static_cast<float>(m_state.frequency) / SAMPLE_RATE;

    // Decode and set breath pattern
    SigilROM::decodeBreathPattern(sigil.breath_pattern, m_breath_pattern);

    // Determine tier from frequency for color
    uint8_t tier = 5;  // Default to middle
    if (sigil.frequency <= 396) tier = (sigil.frequency <= 285) ? (sigil.frequency <= 174 ? 1 : 2) : 3;
    else if (sigil.frequency <= 639) tier = (sigil.frequency <= 528) ? (sigil.frequency <= 417 ? 4 : 5) : 6;
    else tier = (sigil.frequency <= 852) ? (sigil.frequency <= 741 ? 7 : 8) : 9;

    Phase phase = (tier <= 3) ? Phase::UNTRUE : (tier <= 6) ? Phase::PARADOX : Phase::TRUE;
    phaseToColor(phase, m_state.rgb[0], m_state.rgb[1], m_state.rgb[2]);
}

void Emanation::setFrequency(uint16_t freq) {
    m_state.frequency = freq;
    m_audio_phase_inc = static_cast<float>(freq) / SAMPLE_RATE;
}

void Emanation::setWaveform(Waveform wave) {
    m_state.waveform = wave;
}

void Emanation::setVolume(uint8_t vol) {
    m_state.volume = vol;
}

void Emanation::setColor(uint8_t r, uint8_t g, uint8_t b) {
    m_state.rgb[0] = r;
    m_state.rgb[1] = g;
    m_state.rgb[2] = b;
}

void Emanation::setPattern(LedPattern pattern) {
    m_state.pattern = pattern;
}

void Emanation::setBrightness(uint8_t brightness) {
    m_state.brightness = brightness;
    strip.setBrightness(brightness);
}

void Emanation::setOutputMode(uint8_t modes) {
    m_state.output_mode = modes;
    m_state.audio_enabled = (modes & OutputMode::AUDIO) != 0;
    m_state.visual_enabled = (modes & OutputMode::VISUAL) != 0;
    m_state.haptic_enabled = (modes & OutputMode::HAPTIC) != 0;
}

void Emanation::enableAudio(bool enable) {
    m_state.audio_enabled = enable;
    if (enable) {
        m_state.output_mode |= OutputMode::AUDIO;
    } else {
        m_state.output_mode &= ~OutputMode::AUDIO;
    }
}

void Emanation::enableVisual(bool enable) {
    m_state.visual_enabled = enable;
    if (enable) {
        m_state.output_mode |= OutputMode::VISUAL;
    } else {
        m_state.output_mode &= ~OutputMode::VISUAL;
        // Turn off LEDs
        strip.clear();
        strip.show();
    }
}

void Emanation::enableHaptic(bool enable) {
    m_state.haptic_enabled = enable;
    if (enable) {
        m_state.output_mode |= OutputMode::HAPTIC;
    } else {
        m_state.output_mode &= ~OutputMode::HAPTIC;
    }
}

void Emanation::startBreathSync(const BreathPattern& pattern) {
    m_breath_pattern = pattern;
    m_state.breath_sync = true;
    m_state.breath_phase = 0;
    m_state.breath_timer = millis();
}

void Emanation::stopBreathSync() {
    m_state.breath_sync = false;
}

void Emanation::setBinaural(const BinauralConfig& config) {
    m_binaural = config;
    m_state.waveform = Waveform::BINAURAL;

    // Set up phase increments for both frequencies
    m_audio_phase_inc = static_cast<float>(config.base_freq) / SAMPLE_RATE;
    m_binaural_phase_inc = static_cast<float>(config.base_freq + config.beat_freq) / SAMPLE_RATE;
}

void Emanation::stop() {
    m_state.audio_enabled = false;
    m_state.visual_enabled = false;
    m_state.haptic_enabled = false;
    m_state.output_mode = OutputMode::NONE;

    strip.clear();
    strip.show();
}

void Emanation::updateBreathPhase() {
    uint32_t now = millis();
    uint32_t elapsed = now - m_state.breath_timer;

    // Determine which phase we're in based on elapsed time
    uint16_t phase_durations[4] = {
        m_breath_pattern.inhale_ms,
        m_breath_pattern.hold_in_ms,
        m_breath_pattern.exhale_ms,
        m_breath_pattern.hold_out_ms
    };

    uint32_t total = phase_durations[0] + phase_durations[1] +
                     phase_durations[2] + phase_durations[3];

    if (total == 0) return;

    // Wrap elapsed time to cycle duration
    elapsed = elapsed % total;

    // Find current phase
    uint32_t cumulative = 0;
    for (uint8_t i = 0; i < 4; i++) {
        cumulative += phase_durations[i];
        if (elapsed < cumulative) {
            m_state.breath_phase = i;
            break;
        }
    }
}

float Emanation::getBreathModulation() {
    if (!m_state.breath_sync) return 1.0f;

    uint32_t now = millis();
    uint32_t elapsed = now - m_state.breath_timer;

    // Calculate position within current phase
    uint16_t phase_durations[4] = {
        m_breath_pattern.inhale_ms,
        m_breath_pattern.hold_in_ms,
        m_breath_pattern.exhale_ms,
        m_breath_pattern.hold_out_ms
    };

    uint32_t total = phase_durations[0] + phase_durations[1] +
                     phase_durations[2] + phase_durations[3];

    if (total == 0) return 1.0f;

    elapsed = elapsed % total;

    // Calculate modulation based on breath phase
    // Inhale: 0 -> 1, Hold in: 1, Exhale: 1 -> 0, Hold out: 0
    uint32_t cumulative = 0;
    for (uint8_t i = 0; i < 4; i++) {
        if (elapsed < cumulative + phase_durations[i]) {
            float phase_progress = static_cast<float>(elapsed - cumulative) / phase_durations[i];
            switch (i) {
                case 0: return phase_progress;           // Inhale: rising
                case 1: return 1.0f;                     // Hold in: max
                case 2: return 1.0f - phase_progress;    // Exhale: falling
                case 3: return 0.0f;                     // Hold out: min
            }
        }
        cumulative += phase_durations[i];
    }

    return 1.0f;
}

int16_t Emanation::getNextAudioSample() {
    if (!m_state.audio_enabled) {
        return 0;
    }

    int16_t sample = 0;
    float breath_mod = getBreathModulation();

    switch (m_state.waveform) {
        case Waveform::SINE:
            sample = generateSine(m_audio_phase);
            break;
        case Waveform::TRIANGLE:
            sample = generateTriangle(m_audio_phase);
            break;
        case Waveform::SQUARE:
            sample = generateSquare(m_audio_phase);
            break;
        case Waveform::SAWTOOTH:
            sample = static_cast<int16_t>((m_audio_phase * 2.0f - 1.0f) * 32767);
            break;
        case Waveform::BINAURAL:
            // Mix two frequencies for binaural beat
            sample = (generateSine(m_audio_phase) + generateSine(m_binaural_phase)) / 2;
            m_binaural_phase += m_binaural_phase_inc;
            if (m_binaural_phase >= 1.0f) m_binaural_phase -= 1.0f;
            break;
    }

    // Apply volume and breath modulation
    float volume_scale = static_cast<float>(m_state.volume) / 255.0f;
    sample = static_cast<int16_t>(sample * volume_scale * breath_mod);

    // Advance phase
    m_audio_phase += m_audio_phase_inc;
    if (m_audio_phase >= 1.0f) {
        m_audio_phase -= 1.0f;
    }

    return sample;
}

int16_t Emanation::generateSine(float phase) {
    return static_cast<int16_t>(sinf(phase * TWO_PI) * 32767);
}

int16_t Emanation::generateTriangle(float phase) {
    float value;
    if (phase < 0.25f) {
        value = phase * 4.0f;
    } else if (phase < 0.75f) {
        value = 1.0f - (phase - 0.25f) * 4.0f;
    } else {
        value = -1.0f + (phase - 0.75f) * 4.0f;
    }
    return static_cast<int16_t>(value * 32767);
}

int16_t Emanation::generateSquare(float phase) {
    return (phase < 0.5f) ? 32767 : -32767;
}

void Emanation::setPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < Pins::NEOPIXEL_COUNT) {
        strip.setPixelColor(index, strip.Color(r, g, b));
    }
}

void Emanation::updateLeds() {
    if (!m_state.visual_enabled) return;

    uint32_t now = millis();
    float dt = (now - m_led_last_update) / 1000.0f;
    m_led_last_update = now;

    // Advance animation phase
    m_led_anim_phase += dt * 0.5f;  // 0.5 Hz base rate
    if (m_led_anim_phase >= 1.0f) {
        m_led_anim_phase -= 1.0f;
    }

    applyLedPattern();
    strip.show();
}

void Emanation::applyLedPattern() {
    float breath_mod = getBreathModulation();

    switch (m_state.pattern) {
        case LedPattern::SOLID:
            // All LEDs same color
            for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
                setPixel(i,
                    static_cast<uint8_t>(m_state.rgb[0] * breath_mod),
                    static_cast<uint8_t>(m_state.rgb[1] * breath_mod),
                    static_cast<uint8_t>(m_state.rgb[2] * breath_mod));
            }
            break;

        case LedPattern::BREATHE:
            // Smooth breathing effect
            {
                float intensity = (sinf(m_led_anim_phase * TWO_PI) + 1.0f) / 2.0f;
                intensity *= breath_mod;
                for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
                    setPixel(i,
                        static_cast<uint8_t>(m_state.rgb[0] * intensity),
                        static_cast<uint8_t>(m_state.rgb[1] * intensity),
                        static_cast<uint8_t>(m_state.rgb[2] * intensity));
                }
            }
            break;

        case LedPattern::PULSE:
            // Sharp pulse effect
            {
                float pulse = expf(-20.0f * m_led_anim_phase);
                pulse *= breath_mod;
                for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
                    setPixel(i,
                        static_cast<uint8_t>(m_state.rgb[0] * pulse),
                        static_cast<uint8_t>(m_state.rgb[1] * pulse),
                        static_cast<uint8_t>(m_state.rgb[2] * pulse));
                }
            }
            break;

        case LedPattern::WAVE:
            // Wave propagating outward from center
            {
                uint8_t center = Pins::NEOPIXEL_COUNT / 2;
                for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
                    float dist = fabsf(static_cast<float>(i) - center) / center;
                    float wave = sinf((m_led_anim_phase - dist * 0.3f) * TWO_PI);
                    wave = (wave + 1.0f) / 2.0f * breath_mod;
                    setPixel(i,
                        static_cast<uint8_t>(m_state.rgb[0] * wave),
                        static_cast<uint8_t>(m_state.rgb[1] * wave),
                        static_cast<uint8_t>(m_state.rgb[2] * wave));
                }
            }
            break;

        case LedPattern::SPIRAL:
            // Phi-based spiral pattern
            {
                for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
                    float theta = static_cast<float>(i) / Pins::NEOPIXEL_COUNT * TWO_PI;
                    float spiral = sinf(theta * PHI + m_led_anim_phase * TWO_PI);
                    spiral = (spiral + 1.0f) / 2.0f * breath_mod;
                    setPixel(i,
                        static_cast<uint8_t>(m_state.rgb[0] * spiral),
                        static_cast<uint8_t>(m_state.rgb[1] * spiral),
                        static_cast<uint8_t>(m_state.rgb[2] * spiral));
                }
            }
            break;

        case LedPattern::INTERFERENCE:
            generateInterferencePattern(m_led_anim_phase);
            break;

        case LedPattern::SIGIL:
            // Reserved for sigil-specific patterns
            // Default to solid for now
            for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
                setPixel(i, m_state.rgb[0], m_state.rgb[1], m_state.rgb[2]);
            }
            break;
    }
}

void Emanation::generateInterferencePattern(float z) {
    // Simulate interference pattern on linear LED strip
    float f = 0.5f + z * PHI;  // Spatial frequency from z

    for (uint8_t i = 0; i < Pins::NEOPIXEL_COUNT; i++) {
        float pos = static_cast<float>(i) / Pins::NEOPIXEL_COUNT;

        // Reference wave
        float ref = cosf(TWO_PI * f * pos);

        // Object wave (from center)
        float center = 0.5f;
        float r = fabsf(pos - center);
        float obj = cosf(TWO_PI * f * r + m_led_anim_phase * TWO_PI);

        // Interference
        float interference = (ref + obj);
        interference = (interference + 2.0f) / 4.0f;  // Normalize to [0, 1]

        setPixel(i,
            static_cast<uint8_t>(m_state.rgb[0] * interference),
            static_cast<uint8_t>(m_state.rgb[1] * interference),
            static_cast<uint8_t>(m_state.rgb[2] * interference));
    }
}

// Utility functions
void phaseToColor(Phase phase, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t idx = static_cast<uint8_t>(phase);
    if (idx > 2) idx = 0;

    r = PHASE_COLORS[idx][0];
    g = PHASE_COLORS[idx][1];
    b = PHASE_COLORS[idx][2];
}

uint16_t tierToSolfeggio(uint8_t tier) {
    switch (tier) {
        case 1: return Solfeggio::UT;
        case 2: return Solfeggio::RE;
        case 3: return Solfeggio::MI;
        case 4: return Solfeggio::FA;
        case 5: return Solfeggio::SOL;
        case 6: return Solfeggio::LA;
        case 7: return Solfeggio::SI;
        case 8: return Solfeggio::DO;
        case 9: return Solfeggio::RE_HIGH;
        default: return Solfeggio::SOL;
    }
}

void lerpColor(uint8_t r1, uint8_t g1, uint8_t b1,
               uint8_t r2, uint8_t g2, uint8_t b2,
               float t,
               uint8_t& ro, uint8_t& go, uint8_t& bo) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    ro = static_cast<uint8_t>(r1 + (r2 - r1) * t);
    go = static_cast<uint8_t>(g1 + (g2 - g1) * t);
    bo = static_cast<uint8_t>(b1 + (b2 - b1) * t);
}

} // namespace UCF

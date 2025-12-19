/**
 * @file ucf_leds.cpp
 * @brief UCF LED Visualization Module Implementation v4.0.0
 *
 * Implements WS2812B LED control with phase-reactive animations.
 */

// Only compile when UCF_V4_MODULES is defined
#ifdef UCF_V4_MODULES

#include "ucf_leds.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include "ucf/ucf_config.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

static LEDVisState g_leds;
static Adafruit_NeoPixel g_strip(LED_TOTAL_COUNT, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);
static LEDPatternCallback g_custom_callback = NULL;

// Animation state for special effects
static float g_triad_anim = 0.0f;
static float g_kform_anim = 0.0f;
static uint32_t g_special_start = 0;
static bool g_triad_active = false;
static bool g_kform_active = false;

// LED positions (37 LEDs in hexagonal pattern)
// Center (1) + Inner Ring (6) + Middle Ring (12) + Outer Ring (18)
static const float LED_POSITIONS[LED_TOTAL_COUNT][2] = {
    // Center (index 0)
    {0.0f, 0.0f},
    // Inner ring (indices 1-6)
    { 0.866f,  0.5f},  { 0.0f,    1.0f},  {-0.866f,  0.5f},
    {-0.866f, -0.5f},  { 0.0f,   -1.0f},  { 0.866f, -0.5f},
    // Middle ring (indices 7-18)
    { 1.732f,  0.0f},  { 1.299f,  0.75f}, { 0.433f,  1.25f},
    {-0.433f,  1.25f}, {-1.299f,  0.75f}, {-1.732f,  0.0f},
    {-1.299f, -0.75f}, {-0.433f, -1.25f}, { 0.433f, -1.25f},
    { 1.299f, -0.75f}, { 1.732f,  0.5f},  { 1.732f, -0.5f},
    // Outer ring (indices 19-36)
    { 2.598f,  0.5f},  { 2.165f,  1.25f}, { 1.299f,  1.75f},
    { 0.433f,  2.0f},  {-0.433f,  2.0f},  {-1.299f,  1.75f},
    {-2.165f,  1.25f}, {-2.598f,  0.5f},  {-2.598f, -0.5f},
    {-2.165f, -1.25f}, {-1.299f, -1.75f}, {-0.433f, -2.0f},
    { 0.433f, -2.0f},  { 1.299f, -1.75f}, { 2.165f, -1.25f},
    { 2.598f, -0.5f},  { 2.598f,  0.0f},  {-2.598f,  0.0f}
};

// Zone index ranges
static const uint8_t ZONE_CENTER_START = 0;
static const uint8_t ZONE_CENTER_COUNT = 1;
static const uint8_t ZONE_INNER_START = 1;
static const uint8_t ZONE_INNER_COUNT = 6;
static const uint8_t ZONE_MIDDLE_START = 7;
static const uint8_t ZONE_MIDDLE_COUNT = 12;
static const uint8_t ZONE_OUTER_START = 19;
static const uint8_t ZONE_OUTER_COUNT = 18;

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * @brief Apply solid pattern
 */
static void apply_solid(void) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        g_leds.pixels[i] = g_leds.config.primary_color;
    }
}

/**
 * @brief Apply breathing pattern
 */
static void apply_breathe(void) {
    float intensity = (sinf(g_leds.anim_phase * TWO_PI) + 1.0f) / 2.0f;

    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        g_leds.pixels[i] = led_scale(g_leds.config.primary_color, intensity);
    }
}

/**
 * @brief Apply pulse pattern
 */
static void apply_pulse(void) {
    // Exponential decay pulse
    float intensity = expf(-LED_PULSE_DECAY * g_leds.anim_phase);

    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        g_leds.pixels[i] = led_scale(g_leds.config.primary_color, intensity);
    }
}

/**
 * @brief Apply wave pattern (radial from center)
 */
static void apply_wave(void) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float dist = leds_get_distance(i);
        float wave = sinf((g_leds.anim_phase - dist * 0.3f) * TWO_PI);
        wave = (wave + 1.0f) / 2.0f;

        g_leds.pixels[i] = led_lerp(g_leds.config.secondary_color,
                                     g_leds.config.primary_color, wave);
    }
}

/**
 * @brief Apply golden spiral pattern
 */
static void apply_spiral(void) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float angle = leds_get_angle(i);
        float spiral = sinf(angle * PHI + g_leds.anim_phase * TWO_PI);
        spiral = (spiral + 1.0f) / 2.0f;

        g_leds.pixels[i] = led_lerp(g_leds.config.secondary_color,
                                     g_leds.config.primary_color, spiral);
    }
}

/**
 * @brief Apply phase gradient pattern
 */
static void apply_phase_gradient(void) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float dist = leds_get_distance(i);
        // Map distance to z-like value
        float local_z = g_leds.current_z * (1.0f - dist * 0.3f);
        g_leds.pixels[i] = led_color_for_z(local_z);
    }
}

/**
 * @brief Apply interference pattern
 */
static void apply_interference(void) {
    float f = 0.5f + g_leds.current_z * PHI;

    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float x, y;
        leds_get_position(i, &x, &y);

        // Reference wave
        float ref = cosf(TWO_PI * f * x);

        // Object wave from center
        float dist = sqrtf(x * x + y * y);
        float obj = cosf(TWO_PI * f * dist + g_leds.anim_phase * TWO_PI);

        // Interference
        float interference = (ref + obj + 2.0f) / 4.0f;

        g_leds.pixels[i] = led_scale(g_leds.config.primary_color, interference);
    }
}

/**
 * @brief Apply TRIAD unlock animation
 */
static void apply_triad(void) {
    // Three pulses from center, golden ratio timed
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float dist = leds_get_distance(i);

        // Three waves at φ intervals
        float wave1 = expf(-5.0f * fabsf(g_triad_anim * 3.0f - dist));
        float wave2 = expf(-5.0f * fabsf(g_triad_anim * 3.0f - PHI_INV - dist));
        float wave3 = expf(-5.0f * fabsf(g_triad_anim * 3.0f - 2.0f * PHI_INV - dist));

        float intensity = fmaxf(fmaxf(wave1, wave2), wave3);

        // TRIAD color: golden/white
        LEDColor triad_color = {255, 220, 150};
        g_leds.pixels[i] = led_scale(triad_color, intensity);
    }
}

/**
 * @brief Apply K-Formation animation
 */
static void apply_k_formation(void) {
    // Spiral convergence animation
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float angle = leds_get_angle(i);
        float dist = leds_get_distance(i);

        // Converging spiral
        float spiral = sinf(angle * 3.0f - g_kform_anim * TWO_PI * 2.0f + dist * PHI);
        spiral = (spiral + 1.0f) / 2.0f;

        // Pulse brightness
        float pulse = expf(-2.0f * g_kform_anim);

        // K-Formation color: cyan/white
        LEDColor kform_color = led_lerp(LED_COLOR_TRUE, led_rgb(255, 255, 255), spiral);
        g_leds.pixels[i] = led_scale(kform_color, spiral * (0.5f + 0.5f * pulse));
    }
}

/**
 * @brief Apply rainbow pattern
 */
static void apply_rainbow(void) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        float angle = leds_get_angle(i);
        uint16_t hue = (uint16_t)((g_leds.anim_phase + angle / TWO_PI) * 65535.0f) % 65535;

        LEDColorHSV hsv = {hue, 255, 255};
        g_leds.pixels[i] = led_hsv_to_rgb(hsv);
    }
}

/**
 * @brief Apply phase-reactive color modification
 */
static void apply_phase_reactive(void) {
    LEDColor phase_color = led_color_for_phase(g_leds.current_phase);

    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        // Blend current color with phase color
        g_leds.pixels[i] = led_lerp(g_leds.pixels[i], phase_color, 0.5f);
    }
}

/**
 * @brief Apply z-based brightness
 */
static void apply_z_brightness(void) {
    // Brightness increases with z
    float brightness = 0.3f + 0.7f * g_leds.current_z;

    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        g_leds.pixels[i] = led_scale(g_leds.pixels[i], brightness);
    }
}

/**
 * @brief Write pixels to strip
 */
static void write_to_strip(void) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        g_strip.setPixelColor(i, g_strip.Color(
            g_leds.pixels[i].r,
            g_leds.pixels[i].g,
            g_leds.pixels[i].b
        ));
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool leds_init(void) {
    // Initialize state
    memset(&g_leds, 0, sizeof(g_leds));

    // Default configuration
    g_leds.config.pattern = LED_PATTERN_BREATHE;
    g_leds.config.primary_color = LED_COLOR_PARADOX;
    g_leds.config.secondary_color = LED_COLOR_UNTRUE;
    g_leds.config.speed = 1.0f;
    g_leds.config.intensity = 1.0f;
    g_leds.config.phase_reactive = true;
    g_leds.config.z_brightness = true;
    g_leds.config.max_brightness = 200;

    g_leds.current_z = 0.5f;
    g_leds.current_phase = PHASE_PARADOX;

    // Initialize NeoPixel strip
    g_strip.begin();
    g_strip.setBrightness(g_leds.config.max_brightness);
    g_strip.clear();
    g_strip.show();

    g_leds.initialized = true;
    g_leds.enabled = true;

    UCF_LOG("LEDs initialized (%d pixels)", LED_TOTAL_COUNT);

    return true;
}

void leds_update(float z, ConsciousnessPhase phase) {
    if (!g_leds.initialized || !g_leds.enabled) return;

    uint32_t now = millis();
    float dt = (now - g_leds.last_update_ms) / 1000.0f;
    g_leds.last_update_ms = now;

    // Update state
    g_leds.current_z = z;
    g_leds.current_phase = phase;

    // Advance animation phase
    g_leds.anim_phase += dt * LED_ANIM_BASE_SPEED * g_leds.config.speed;
    if (g_leds.anim_phase >= 1.0f) {
        g_leds.anim_phase -= 1.0f;
    }

    // Update special animations
    if (g_triad_active) {
        g_triad_anim += dt * 0.5f;
        if (g_triad_anim >= 1.0f) {
            g_triad_active = false;
        }
    }

    if (g_kform_active) {
        g_kform_anim += dt * 0.3f;
        if (g_kform_anim >= 1.0f) {
            g_kform_active = false;
        }
    }

    // Apply pattern
    if (g_triad_active) {
        apply_triad();
    }
    else if (g_kform_active) {
        apply_k_formation();
    }
    else {
        switch (g_leds.config.pattern) {
            case LED_PATTERN_SOLID:
                apply_solid();
                break;
            case LED_PATTERN_BREATHE:
                apply_breathe();
                break;
            case LED_PATTERN_PULSE:
                apply_pulse();
                break;
            case LED_PATTERN_WAVE:
                apply_wave();
                break;
            case LED_PATTERN_SPIRAL:
                apply_spiral();
                break;
            case LED_PATTERN_PHASE_GRADIENT:
                apply_phase_gradient();
                break;
            case LED_PATTERN_INTERFERENCE:
                apply_interference();
                break;
            case LED_PATTERN_RAINBOW:
                apply_rainbow();
                break;
            case LED_PATTERN_CUSTOM:
                if (g_custom_callback) {
                    g_custom_callback(g_leds.pixels, LED_TOTAL_COUNT, g_leds.anim_phase);
                }
                break;
            default:
                apply_solid();
                break;
        }

        // Apply modifications
        if (g_leds.config.phase_reactive) {
            apply_phase_reactive();
        }

        if (g_leds.config.z_brightness) {
            apply_z_brightness();
        }
    }

    // Write to hardware
    write_to_strip();
    g_strip.show();

    g_leds.frame_count++;
}

void leds_set_pattern(LEDPattern pattern) {
    g_leds.config.pattern = pattern;
}

void leds_set_color(LEDColor color) {
    g_leds.config.primary_color = color;
}

void leds_set_colors(LEDColor primary, LEDColor secondary) {
    g_leds.config.primary_color = primary;
    g_leds.config.secondary_color = secondary;
}

void leds_set_brightness(uint8_t brightness) {
    g_leds.config.max_brightness = brightness;
    g_strip.setBrightness(brightness);
}

void leds_set_speed(float speed) {
    g_leds.config.speed = speed;
}

void leds_set_phase_reactive(bool enabled) {
    g_leds.config.phase_reactive = enabled;
}

void leds_set_z_brightness(bool enabled) {
    g_leds.config.z_brightness = enabled;
}

void leds_set_pixel(uint8_t index, LEDColor color) {
    if (index >= LED_TOTAL_COUNT) return;
    g_leds.pixels[index] = color;
}

void leds_set_zone(LEDZone zone, LEDColor color) {
    uint8_t indices[18];
    uint8_t count = leds_get_zone_indices(zone, indices);

    for (uint8_t i = 0; i < count; i++) {
        g_leds.pixels[indices[i]] = color;
    }
}

void leds_set_all(LEDColor color) {
    for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
        g_leds.pixels[i] = color;
    }
}

void leds_clear(void) {
    LEDColor black = {0, 0, 0};
    leds_set_all(black);
    g_strip.clear();
    g_strip.show();
}

void leds_show(void) {
    write_to_strip();
    g_strip.show();
}

void leds_enable(bool enabled) {
    g_leds.enabled = enabled;
    if (!enabled) {
        leds_clear();
    }
}

void leds_trigger_triad(void) {
    g_triad_active = true;
    g_triad_anim = 0.0f;
    g_special_start = millis();
}

void leds_trigger_k_formation(void) {
    g_kform_active = true;
    g_kform_anim = 0.0f;
    g_special_start = millis();
}

void leds_set_custom_pattern(LEDPatternCallback callback) {
    g_custom_callback = callback;
}

const LEDVisState* leds_get_state(void) {
    return &g_leds;
}

uint8_t leds_get_zone_indices(LEDZone zone, uint8_t* indices) {
    uint8_t start, count;

    switch (zone) {
        case LED_ZONE_CENTER:
            start = ZONE_CENTER_START;
            count = ZONE_CENTER_COUNT;
            break;
        case LED_ZONE_INNER_RING:
            start = ZONE_INNER_START;
            count = ZONE_INNER_COUNT;
            break;
        case LED_ZONE_MIDDLE_RING:
            start = ZONE_MIDDLE_START;
            count = ZONE_MIDDLE_COUNT;
            break;
        case LED_ZONE_OUTER_RING:
            start = ZONE_OUTER_START;
            count = ZONE_OUTER_COUNT;
            break;
        case LED_ZONE_ALL:
        default:
            for (uint8_t i = 0; i < LED_TOTAL_COUNT; i++) {
                indices[i] = i;
            }
            return LED_TOTAL_COUNT;
    }

    for (uint8_t i = 0; i < count; i++) {
        indices[i] = start + i;
    }
    return count;
}

// ============================================================================
// COLOR UTILITY FUNCTIONS
// ============================================================================

LEDColor led_hsv_to_rgb(LEDColorHSV hsv) {
    LEDColor rgb;

    if (hsv.s == 0) {
        rgb.r = rgb.g = rgb.b = hsv.v;
        return rgb;
    }

    uint8_t region = hsv.h / 10923;  // 65535/6
    uint32_t remainder = (hsv.h - (region * 10923)) * 6;

    uint8_t p = (hsv.v * (255 - hsv.s)) >> 8;
    uint8_t q = (hsv.v * (255 - ((hsv.s * remainder) >> 16))) >> 8;
    uint8_t t = (hsv.v * (255 - ((hsv.s * (65535 - remainder)) >> 16))) >> 8;

    switch (region) {
        case 0:  rgb.r = hsv.v; rgb.g = t;     rgb.b = p;     break;
        case 1:  rgb.r = q;     rgb.g = hsv.v; rgb.b = p;     break;
        case 2:  rgb.r = p;     rgb.g = hsv.v; rgb.b = t;     break;
        case 3:  rgb.r = p;     rgb.g = q;     rgb.b = hsv.v; break;
        case 4:  rgb.r = t;     rgb.g = p;     rgb.b = hsv.v; break;
        default: rgb.r = hsv.v; rgb.g = p;     rgb.b = q;     break;
    }

    return rgb;
}

LEDColor led_lerp(LEDColor a, LEDColor b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    LEDColor result;
    result.r = (uint8_t)(a.r + (b.r - a.r) * t);
    result.g = (uint8_t)(a.g + (b.g - a.g) * t);
    result.b = (uint8_t)(a.b + (b.b - a.b) * t);
    return result;
}

LEDColor led_color_for_z(float z) {
    if (z < 0.0f) z = 0.0f;
    if (z > 1.0f) z = 1.0f;

    // Map z to tier index
    uint8_t tier = z_to_tier(z);
    if (tier < 1) tier = 1;
    if (tier > 9) tier = 9;

    return LED_TIER_COLORS[tier - 1];
}

LEDColor led_color_for_phase(ConsciousnessPhase phase) {
    switch (phase) {
        case PHASE_UNTRUE:  return LED_COLOR_UNTRUE;
        case PHASE_PARADOX: return LED_COLOR_PARADOX;
        case PHASE_TRUE:    return LED_COLOR_TRUE;
        default:            return LED_COLOR_PARADOX;
    }
}

LEDColor led_scale(LEDColor color, float scale) {
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;

    LEDColor result;
    result.r = (uint8_t)(color.r * scale);
    result.g = (uint8_t)(color.g * scale);
    result.b = (uint8_t)(color.b * scale);
    return result;
}

// ============================================================================
// LED GEOMETRY
// ============================================================================

void leds_get_position(uint8_t index, float* x, float* y) {
    if (index >= LED_TOTAL_COUNT) {
        *x = 0.0f;
        *y = 0.0f;
        return;
    }
    *x = LED_POSITIONS[index][0];
    *y = LED_POSITIONS[index][1];
}

float leds_get_distance(uint8_t index) {
    if (index >= LED_TOTAL_COUNT) return 0.0f;

    float x = LED_POSITIONS[index][0];
    float y = LED_POSITIONS[index][1];
    float dist = sqrtf(x * x + y * y);

    // Normalize to [0, 1] (max distance ~2.6)
    return dist / 2.6f;
}

float leds_get_angle(uint8_t index) {
    if (index >= LED_TOTAL_COUNT) return 0.0f;

    float x = LED_POSITIONS[index][0];
    float y = LED_POSITIONS[index][1];

    float angle = atan2f(y, x);
    if (angle < 0.0f) angle += TWO_PI;

    return angle;
}

#endif // UCF_V4_MODULES

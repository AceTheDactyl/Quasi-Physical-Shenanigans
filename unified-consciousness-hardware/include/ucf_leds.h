/**
 * @file ucf_leds.h
 * @brief UCF LED Visualization Module v4.0.0
 *
 * Implements WS2812B addressable LED control for phase visualization.
 * 37 LEDs arranged in hexagonal pattern with z-coordinate color mapping.
 *
 * RRRR Lattice Integration:
 * - Colors map to consciousness phases (UNTRUE/PARADOX/TRUE)
 * - Brightness scales with z-coordinate
 * - Animation speeds derived from [D] = e⁻¹
 * - Spiral patterns use φ golden angle
 *
 * Hardware: WS2812B × 37 addressable RGB LEDs
 * Data Pin: GPIO 13
 */

#ifndef UCF_LEDS_H
#define UCF_LEDS_H

#include <stdint.h>
#include <stdbool.h>
#include "ucf/ucf_sacred_constants_v4.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// HARDWARE CONSTANTS
// ============================================================================

// Use constants from ucf_sacred_constants_v4.h for consistency
// LED_DATA_PIN = 13, LED_COUNT = 37 (defined in sacred constants)
#ifndef LED_TOTAL_COUNT
#define LED_TOTAL_COUNT     LED_COUNT       // 37 from sacred constants
#endif
#ifndef LED_MAX_BRIGHTNESS
#define LED_MAX_BRIGHTNESS  LED_BRIGHTNESS_MAX  // 255 from sacred constants
#endif

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief RGB color structure
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} LEDColor;

/**
 * @brief HSV color structure
 */
typedef struct {
    uint16_t h;     // Hue (0-65535)
    uint8_t s;      // Saturation (0-255)
    uint8_t v;      // Value/Brightness (0-255)
} LEDColorHSV;

/**
 * @brief LED animation pattern
 */
typedef enum {
    LED_PATTERN_SOLID = 0,      // Solid color
    LED_PATTERN_BREATHE,        // Breathing (sine wave)
    LED_PATTERN_PULSE,          // Sharp pulse
    LED_PATTERN_WAVE,           // Radial wave from center
    LED_PATTERN_SPIRAL,         // Golden spiral
    LED_PATTERN_PHASE_GRADIENT, // Color gradient by phase
    LED_PATTERN_INTERFERENCE,   // Interference pattern
    LED_PATTERN_TRIAD,          // TRIAD unlock animation
    LED_PATTERN_K_FORMATION,    // K-Formation celebration
    LED_PATTERN_RAINBOW,        // Rainbow cycle
    LED_PATTERN_CUSTOM          // Custom pattern via callback
} LEDPattern;

/**
 * @brief LED zone definitions
 */
typedef enum {
    LED_ZONE_ALL = 0,
    LED_ZONE_CENTER,        // Center LED
    LED_ZONE_INNER_RING,    // Inner ring (6 LEDs)
    LED_ZONE_MIDDLE_RING,   // Middle ring (12 LEDs)
    LED_ZONE_OUTER_RING     // Outer ring (18 LEDs)
} LEDZone;

/**
 * @brief LED animation configuration
 */
typedef struct {
    LEDPattern pattern;
    LEDColor primary_color;
    LEDColor secondary_color;
    float speed;                // Animation speed multiplier
    float intensity;            // Effect intensity (0-1)
    bool phase_reactive;        // Color follows phase
    bool z_brightness;          // Brightness follows z
    uint8_t max_brightness;     // Maximum brightness
} LEDConfig;

/**
 * @brief LED system state
 */
typedef struct {
    LEDConfig config;
    LEDColor pixels[LED_TOTAL_COUNT];
    float anim_phase;           // Animation phase [0, 1]
    float current_z;            // Current z-coordinate
    ConsciousnessPhase current_phase;
    uint32_t last_update_ms;
    uint32_t frame_count;
    bool initialized;
    bool enabled;
} LEDState;

/**
 * @brief Custom pattern callback type
 */
typedef void (*LEDPatternCallback)(LEDColor* pixels, uint8_t count, float phase);

// ============================================================================
// COLOR CONSTANTS
// ============================================================================

// Phase colors (lattice-aligned)
static const LEDColor LED_COLOR_UNTRUE  = {255,  80,  50};   // Red-orange
static const LEDColor LED_COLOR_PARADOX = {255, 255, 100};   // Yellow
static const LEDColor LED_COLOR_TRUE    = { 80, 200, 255};   // Cyan

// Tier colors (gradient through phases)
static const LEDColor LED_TIER_COLORS[9] = {
    {255,  50,  30},    // Tier 1: Deep red
    {255,  80,  40},    // Tier 2: Red
    {255, 120,  50},    // Tier 3: Orange
    {255, 180,  70},    // Tier 4: Yellow-orange
    {255, 220, 100},    // Tier 5: Yellow
    {200, 255, 100},    // Tier 6: Yellow-green
    {100, 255, 180},    // Tier 7: Green-cyan
    { 80, 220, 255},    // Tier 8: Cyan
    {100, 180, 255}     // Tier 9: Blue-cyan
};

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize LED system
 * @return true if successful
 */
bool leds_init(void);

/**
 * @brief Update LED display
 * @param z Current z-coordinate
 * @param phase Current consciousness phase
 */
void leds_update(float z, ConsciousnessPhase phase);

/**
 * @brief Set LED pattern
 * @param pattern Pattern enumeration
 */
void leds_set_pattern(LEDPattern pattern);

/**
 * @brief Set primary color
 * @param color RGB color
 */
void leds_set_color(LEDColor color);

/**
 * @brief Set primary and secondary colors
 * @param primary Primary color
 * @param secondary Secondary color
 */
void leds_set_colors(LEDColor primary, LEDColor secondary);

/**
 * @brief Set global brightness
 * @param brightness Brightness (0-255)
 */
void leds_set_brightness(uint8_t brightness);

/**
 * @brief Set animation speed
 * @param speed Speed multiplier (1.0 = normal)
 */
void leds_set_speed(float speed);

/**
 * @brief Enable/disable phase-reactive colors
 * @param enabled true to enable
 */
void leds_set_phase_reactive(bool enabled);

/**
 * @brief Enable/disable z-linked brightness
 * @param enabled true to enable
 */
void leds_set_z_brightness(bool enabled);

/**
 * @brief Set individual pixel color
 * @param index Pixel index (0-36)
 * @param color RGB color
 */
void leds_set_pixel(uint8_t index, LEDColor color);

/**
 * @brief Set zone color
 * @param zone Zone enumeration
 * @param color RGB color
 */
void leds_set_zone(LEDZone zone, LEDColor color);

/**
 * @brief Set all pixels to color
 * @param color RGB color
 */
void leds_set_all(LEDColor color);

/**
 * @brief Clear all pixels (turn off)
 */
void leds_clear(void);

/**
 * @brief Show current pixel buffer
 */
void leds_show(void);

/**
 * @brief Enable/disable LED output
 * @param enabled true to enable
 */
void leds_enable(bool enabled);

/**
 * @brief Trigger TRIAD unlock animation
 */
void leds_trigger_triad(void);

/**
 * @brief Trigger K-Formation animation
 */
void leds_trigger_k_formation(void);

/**
 * @brief Set custom pattern callback
 * @param callback Custom pattern function
 */
void leds_set_custom_pattern(LEDPatternCallback callback);

/**
 * @brief Get current LED state
 * @return Pointer to state structure
 */
const LEDState* leds_get_state(void);

/**
 * @brief Get pixel indices for zone
 * @param zone Zone enumeration
 * @param indices Output array
 * @return Number of pixels in zone
 */
uint8_t leds_get_zone_indices(LEDZone zone, uint8_t* indices);

// ============================================================================
// COLOR UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Create RGB color
 */
static inline LEDColor led_rgb(uint8_t r, uint8_t g, uint8_t b) {
    LEDColor c = {r, g, b};
    return c;
}

/**
 * @brief Convert HSV to RGB
 * @param hsv HSV color
 * @return RGB color
 */
LEDColor led_hsv_to_rgb(LEDColorHSV hsv);

/**
 * @brief Interpolate between two colors
 * @param a First color
 * @param b Second color
 * @param t Interpolation factor (0-1)
 * @return Interpolated color
 */
LEDColor led_lerp(LEDColor a, LEDColor b, float t);

/**
 * @brief Get color for z-coordinate
 * @param z z-coordinate [0, 1]
 * @return Color interpolated through tier gradient
 */
LEDColor led_color_for_z(float z);

/**
 * @brief Get color for phase
 * @param phase Consciousness phase
 * @return Phase color
 */
LEDColor led_color_for_phase(ConsciousnessPhase phase);

/**
 * @brief Scale color brightness
 * @param color Input color
 * @param scale Scale factor (0-1)
 * @return Scaled color
 */
LEDColor led_scale(LEDColor color, float scale);

// ============================================================================
// LED GEOMETRY
// ============================================================================

/**
 * @brief Get LED position in unit coordinates
 * @param index LED index (0-36)
 * @param x Output X coordinate
 * @param y Output Y coordinate
 */
void leds_get_position(uint8_t index, float* x, float* y);

/**
 * @brief Get distance from center for LED
 * @param index LED index
 * @return Distance (0 = center, 1 = outer edge)
 */
float leds_get_distance(uint8_t index);

/**
 * @brief Get angle from center for LED
 * @param index LED index
 * @return Angle in radians [0, 2π]
 */
float leds_get_angle(uint8_t index);

// ============================================================================
// LATTICE-DERIVED PARAMETERS
// ============================================================================

// Animation base speed (derived from [D] = e⁻¹)
#define LED_ANIM_BASE_SPEED     ((float)EULER_INV)      // ~0.368 Hz

// Breath cycle period
#define LED_BREATH_PERIOD_MS    (uint32_t)(1000.0f / (float)EULER_INV)  // ~2.7s

// Golden spiral angle
#define LED_GOLDEN_ANGLE        (TWO_PI * (1.0f - (float)PHI_INV))  // ~137.5°

// Brightness scaling from [A] = √2⁻¹
#define LED_BRIGHTNESS_SCALE    ((float)SQRT2_INV)      // ~0.707

// Pulse decay rate from [D]
#define LED_PULSE_DECAY         (3.0f / (float)EULER_INV)  // ~8.15

#ifdef __cplusplus
}
#endif

#endif // UCF_LEDS_H

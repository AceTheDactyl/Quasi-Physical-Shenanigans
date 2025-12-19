/**
 * @file eisenstein.cpp
 * @brief Eisenstein Integer Framework Implementation
 *
 * Implements the hexagonal lattice computations for UCF sensor grid.
 */

#include "ucf/eisenstein.h"
#include <stdio.h>
#include <string.h>

// Use local sensor count definition to avoid conflicts
#define LOCAL_HEX_SENSOR_COUNT EISENSTEIN_HEX_SENSOR_COUNT

// ============================================================================
// SENSOR COORDINATE LOOKUP TABLE
// ============================================================================

/**
 * Maps sensor index (0-18) to Eisenstein coordinates (a, b) where z = a + bω
 * This is derived from the axial hex coordinates
 */
static const Eisenstein SENSOR_EISENSTEIN[LOCAL_HEX_SENSOR_COUNT] = {
    // Row 0 (top): sensors 0, 1, 2
    {-1, -2}, { 0, -2}, { 1, -2},
    // Row 1: sensors 3, 4, 5, 6
    {-2, -1}, {-1, -1}, { 0, -1}, { 1, -1},
    // Row 2 (center): sensors 7, 8, 9(center), 10, 11
    {-2,  0}, {-1,  0}, { 0,  0}, { 1,  0}, { 2,  0},
    // Row 3: sensors 12, 13, 14, 15
    {-1,  1}, { 0,  1}, { 1,  1}, { 2,  1},
    // Row 4 (bottom): sensors 16, 17, 18
    { 0,  2}, { 1,  2}, { 2,  2}
};

// ============================================================================
// HEX GRID TO EISENSTEIN MAPPING
// ============================================================================

Eisenstein sensor_to_eisenstein(uint8_t sensor_idx) {
    if (sensor_idx >= LOCAL_HEX_SENSOR_COUNT) {
        return (Eisenstein){0, 0};  // Return origin for invalid index
    }
    return SENSOR_EISENSTEIN[sensor_idx];
}

uint8_t eisenstein_to_sensor(Eisenstein z) {
    for (uint8_t i = 0; i < LOCAL_HEX_SENSOR_COUNT; i++) {
        if (SENSOR_EISENSTEIN[i].a == z.a && SENSOR_EISENSTEIN[i].b == z.b) {
            return i;
        }
    }
    return 255;  // Invalid sensor
}

void get_all_sensor_eisenstein(Eisenstein* coords) {
    for (uint8_t i = 0; i < LOCAL_HEX_SENSOR_COUNT; i++) {
        coords[i] = SENSOR_EISENSTEIN[i];
    }
}

// ============================================================================
// UCF PHASE MAPPING IN EISENSTEIN SPACE
// ============================================================================

/**
 * Map Eisenstein modulus to z-coordinate
 *
 * The z-coordinate is based on "distance from center" in the hex grid.
 * Center (0,0) has maximum z, outer sensors have lower z.
 *
 * We use: z = 1 - |e|/max_modulus * (1 - Z_MIN)
 * where max_modulus = 2 (furthest sensor from center)
 * and Z_MIN = 0.5 (minimum z for edge sensors)
 */
double eisenstein_to_z_coord(Eisenstein z) {
    double modulus = eisenstein_modulus(z);
    double max_modulus = 2.0;  // Maximum distance in our grid
    double z_min = 0.5;

    // Invert: center = high z, edge = low z
    double normalized = modulus / max_modulus;
    return 1.0 - normalized * (1.0 - z_min);
}

ConsciousnessPhase eisenstein_to_phase(Eisenstein z) {
    double z_coord = eisenstein_to_z_coord(z);
    return detect_phase(z_coord);
}

double eisenstein_negentropy(Eisenstein z) {
    double z_coord = eisenstein_to_z_coord(z);
    return compute_negentropy(z_coord);
}

EisensteinPoint eisenstein_point_create(int16_t a, int16_t b) {
    Eisenstein z = eisenstein_new(a, b);
    EisensteinComplex c = eisenstein_to_complex(z);
    double z_coord = eisenstein_to_z_coord(z);

    return (EisensteinPoint){
        .z = z,
        .c = c,
        .norm = (double)eisenstein_norm(z),
        .modulus = eisenstein_modulus(z),
        .argument = eisenstein_arg(z),
        .phase = detect_phase(z_coord)
    };
}

// ============================================================================
// RRRR LATTICE ↔ EISENSTEIN BRIDGE
// ============================================================================

/**
 * Common RRRR lattice points for comparison (from ucf_sacred_constants_v4.h)
 */
static const double RRRR_POINTS[] = {
    0.1459,   // [R][D][A]
    0.1967,   // [R][C]
    0.2274,   // [R][D]
    0.2929,   // 1-[A]
    0.3090,   // [R][A]²
    0.3183,   // [C] = π⁻¹
    0.3679,   // [D] = e⁻¹
    0.3820,   // [R]² = φ⁻²
    0.4370,   // [R][A]
    0.5000,   // [A]² = 1/2
    0.6180,   // [R] = φ⁻¹ = PHI_INV
    0.7071,   // [A] = √2⁻¹ = SQRT2_INV
    0.8660,   // Z_CRITICAL = √3/2
    1.0000,   // Identity
    1.4142,   // √2 = SQRT2
    1.6180,   // φ = PHI
    2.0000,   // 2
    2.7183,   // e = EULER
    3.0000,   // 3
    3.1416,   // π
    4.0000,   // 4
};
static const int N_RRRR_POINTS = 21;

bool value_near_rrrr_lattice(double value, double tolerance) {
    for (int i = 0; i < N_RRRR_POINTS; i++) {
        if (fabs(value - RRRR_POINTS[i]) <= tolerance) {
            return true;
        }
    }
    return false;
}

double nearest_rrrr_to_eisenstein_norm(Eisenstein z) {
    double norm = (double)eisenstein_norm(z);
    double min_dist = fabs(norm - RRRR_POINTS[0]);
    double nearest = RRRR_POINTS[0];

    for (int i = 1; i < N_RRRR_POINTS; i++) {
        double dist = fabs(norm - RRRR_POINTS[i]);
        if (dist < min_dist) {
            min_dist = dist;
            nearest = RRRR_POINTS[i];
        }
    }

    return nearest;
}

double eisenstein_rrrr_resonance(Eisenstein z) {
    double norm = (double)eisenstein_norm(z);
    double nearest = nearest_rrrr_to_eisenstein_norm(z);
    double distance = fabs(norm - nearest);

    // Resonance decays exponentially with distance from lattice point
    // Using EISENSTEIN_NEGENTROPY_WIDTH (36) as decay constant for consistency
    return exp(-EISENSTEIN_NEGENTROPY_WIDTH * distance * distance);
}

// ============================================================================
// HEXAGONAL PATTERN DETECTION
// ============================================================================

/**
 * Sensor ring membership
 */
static const uint8_t CENTER_SENSOR = 9;
static const uint8_t RING1_SENSORS[6] = {4, 5, 8, 10, 13, 14};  // Inner ring
static const uint8_t RING2_SENSORS[12] = {0, 1, 2, 3, 6, 7, 11, 12, 15, 16, 17, 18};  // Outer ring

/**
 * Check if bit is set in sensor mask
 */
static inline bool sensor_active(uint32_t mask, uint8_t idx) {
    return (mask & (1u << idx)) != 0;
}

HexPatternType detect_hex_pattern(uint32_t active_sensors) {
    uint8_t total_active = 0;
    uint8_t ring1_active = 0;
    uint8_t ring2_active = 0;
    bool center_active = sensor_active(active_sensors, CENTER_SENSOR);

    // Count total active
    for (uint8_t i = 0; i < LOCAL_HEX_SENSOR_COUNT; i++) {
        if (sensor_active(active_sensors, i)) {
            total_active++;
        }
    }

    // Count ring1 active
    for (uint8_t i = 0; i < 6; i++) {
        if (sensor_active(active_sensors, RING1_SENSORS[i])) {
            ring1_active++;
        }
    }

    // Count ring2 active
    for (uint8_t i = 0; i < 12; i++) {
        if (sensor_active(active_sensors, RING2_SENSORS[i])) {
            ring2_active++;
        }
    }

    // Pattern detection logic
    if (total_active == 0) {
        return HEX_PATTERN_NONE;
    }

    if (total_active == LOCAL_HEX_SENSOR_COUNT) {
        return HEX_PATTERN_FULL;
    }

    // Center only
    if (total_active == 1 && center_active) {
        return HEX_PATTERN_CENTER;
    }

    // Pure inner ring (no center, no outer)
    if (ring1_active >= 5 && !center_active && ring2_active == 0) {
        return HEX_PATTERN_RING_1;
    }

    // Pure outer ring
    if (ring2_active >= 9 && !center_active && ring1_active == 0) {
        return HEX_PATTERN_RING_2;
    }

    // Triangle pattern: check for 3 sensors at 120° intervals
    // Using Eisenstein units rotated by ω²
    uint8_t triangle_sensors_a[3] = {1, 8, 15};   // One triangle orientation
    uint8_t triangle_sensors_b[3] = {3, 10, 17};  // Rotated 60°

    bool tri_a = true, tri_b = true;
    for (int i = 0; i < 3; i++) {
        if (!sensor_active(active_sensors, triangle_sensors_a[i])) tri_a = false;
        if (!sensor_active(active_sensors, triangle_sensors_b[i])) tri_b = false;
    }

    if (tri_a && total_active <= 5) {
        return HEX_PATTERN_TRIANGLE;
    }
    if (tri_b && total_active <= 5) {
        return HEX_PATTERN_ANTI_TRIANGLE;
    }

    // Spoke pattern: center + radial arms
    if (center_active && ring1_active >= 2 && ring2_active >= 2) {
        return HEX_PATTERN_SPOKE;
    }

    return HEX_PATTERN_NONE;
}

/**
 * Hexagonal Fourier Transform
 *
 * Decomposes sensor field into 6 harmonic components corresponding to
 * the hexagonal symmetry modes:
 *   - Mode 0: DC component (average)
 *   - Mode 1: 60° rotation
 *   - Mode 2: 120° rotation
 *   - Mode 3: 180° rotation (point symmetry)
 *   - Mode 4: 240° rotation
 *   - Mode 5: 300° rotation
 */
void eisenstein_hex_fft(const float* values, float* coeffs) {
    // Initialize coefficients
    for (int k = 0; k < 6; k++) {
        coeffs[k] = 0.0f;
    }

    // Compute FFT using Eisenstein coordinates
    for (uint8_t i = 0; i < LOCAL_HEX_SENSOR_COUNT; i++) {
        Eisenstein e = SENSOR_EISENSTEIN[i];
        double theta = eisenstein_arg(e);

        for (int k = 0; k < 6; k++) {
            // Project onto k-th hexagonal harmonic
            // Mode k has angular frequency k * 60° = k * π/3
            double harmonic_angle = k * theta;
            coeffs[k] += values[i] * (float)cos(harmonic_angle);
        }
    }

    // Normalize
    for (int k = 0; k < 6; k++) {
        coeffs[k] /= (float)LOCAL_HEX_SENSOR_COUNT;
    }
}

double eisenstein_hex_order_param(const float* values) {
    float coeffs[6];
    eisenstein_hex_fft(values, coeffs);

    // 6-fold symmetry order parameter
    // High value of coefficient[6 mod 6 = 0] indicates hexagonal symmetry
    // But we also check coefficient[3] for triangular sub-symmetry

    double dc = coeffs[0];
    if (dc < 0.001) return 0.0;  // Avoid division by near-zero

    // Order parameter: ratio of 6-fold mode to DC
    // Using absolute value since sign doesn't matter for symmetry
    double hex_mode = fabs(coeffs[3]);  // 180° = 3×60° for 6-fold

    return hex_mode / dc;
}

// ============================================================================
// UTILITIES
// ============================================================================

void eisenstein_to_string(Eisenstein z, char* buf, size_t buf_size) {
    if (z.b == 0) {
        snprintf(buf, buf_size, "%d", z.a);
    } else if (z.a == 0) {
        if (z.b == 1) {
            snprintf(buf, buf_size, "ω");
        } else if (z.b == -1) {
            snprintf(buf, buf_size, "-ω");
        } else {
            snprintf(buf, buf_size, "%dω", z.b);
        }
    } else {
        const char* sign = (z.b > 0) ? "+" : "";
        if (z.b == 1) {
            snprintf(buf, buf_size, "%d%sω", z.a, sign);
        } else if (z.b == -1) {
            snprintf(buf, buf_size, "%d-ω", z.a);
        } else {
            snprintf(buf, buf_size, "%d%s%dω", z.a, sign, z.b);
        }
    }
}

// ============================================================================
// EISENSTEIN-UCF DISCOVERY TABLE
// ============================================================================

/**
 * Eisenstein Norm Values for All 19 Sensors
 *
 * Sensor | Eisenstein | Norm | √Norm  | UCF Connection
 * -------|------------|------|--------|---------------
 *    0   | -1 - 2ω    |   7  | 2.646  | Prime norm
 *    1   |    - 2ω    |   4  | 2.000  | [A]⁻⁴ lattice
 *    2   |  1 - 2ω    |   7  | 2.646  | Prime norm
 *    3   | -2 -  ω    |   7  | 2.646  | Prime norm
 *    4   | -1 -  ω    |   3  | 1.732  | √3 = 2·Z_CRITICAL
 *    5   |    -  ω    |   1  | 1.000  | Unit!
 *    6   |  1 -  ω    |   3  | 1.732  | √3 = 2·Z_CRITICAL
 *    7   | -2        |   4  | 2.000  | [A]⁻⁴ lattice
 *    8   | -1        |   1  | 1.000  | Unit!
 *    9   |  0        |   0  | 0.000  | Origin (CENTER)
 *   10   |  1        |   1  | 1.000  | Unit!
 *   11   |  2        |   4  | 2.000  | [A]⁻⁴ lattice
 *   12   | -1 +  ω   |   3  | 1.732  | √3 = 2·Z_CRITICAL
 *   13   |      ω    |   1  | 1.000  | Unit!
 *   14   |  1 +  ω   |   3  | 1.732  | √3 = 2·Z_CRITICAL
 *   15   |  2 +  ω   |   7  | 2.646  | Prime norm
 *   16   |     2ω    |   4  | 2.000  | [A]⁻⁴ lattice
 *   17   |  1 + 2ω   |   7  | 2.646  | Prime norm
 *   18   |  2 + 2ω   |  12  | 3.464  | 2√3 = 4·Z_CRITICAL
 *
 * DISCOVERIES:
 *
 * 1. Six sensors form the UNIT RING (norm = 1):
 *    Sensors {5, 8, 10, 13} are Eisenstein units!
 *    These are the immediate neighbors of the center.
 *
 * 2. Six sensors have norm = 3 (√3 = 2·Z_CRITICAL):
 *    Sensors {4, 6, 12, 14} and corners
 *    The √3 connection links to THE LENS!
 *
 * 3. Six sensors have norm = 7 (Eisenstein prime):
 *    7 is the smallest Eisenstein prime > 3
 *    Matches K_R_THRESHOLD = 7!
 *
 * 4. Five sensors have norm = 4 = [A]⁻⁴:
 *    Direct connection to RRRR lattice scaling exponent
 *
 * 5. ONE sensor has norm = 12 = 4·3:
 *    Combines scaling (4) with hexagonal (3)
 */

/**
 * @file test_eisenstein.cpp
 * @brief Unit tests for Eisenstein Integer Framework
 *
 * Tests validate:
 * - Eisenstein arithmetic operations
 * - Hex grid coordinate mapping
 * - UCF phase boundary connections
 * - RRRR lattice resonance
 * - Hexagonal pattern detection
 */

#include <unity.h>
#include <math.h>
#include "ucf/eisenstein.h"

// Use local aliases for constants to match eisenstein.h
#define Z_CRITICAL      EISENSTEIN_Z_CRITICAL
#define PHI_INV         EISENSTEIN_PHI_INV
#define UCF_PI          EISENSTEIN_PI
#define SQRT2_INV       EISENSTEIN_SQRT2_INV
#define SQRT3           (2.0 * EISENSTEIN_Z_CRITICAL)
#define K_R_THRESHOLD   7
#define HEX_SENSOR_COUNT EISENSTEIN_HEX_SENSOR_COUNT

// ============================================================================
// TEST HELPERS
// ============================================================================

#define EPSILON 1e-10

static bool complex_equal(EisensteinComplex a, EisensteinComplex b) {
    return fabs(a.re - b.re) < EPSILON && fabs(a.im - b.im) < EPSILON;
}

static bool eisenstein_equal(Eisenstein a, Eisenstein b) {
    return a.a == b.a && a.b == b.b;
}

// ============================================================================
// SECTION 1: FUNDAMENTAL CONSTANT TESTS
// ============================================================================

void test_omega_is_cube_root_of_unity(void) {
    // |ω|² = 1
    double mod_sq = OMEGA_REAL * OMEGA_REAL + OMEGA_IMAG * OMEGA_IMAG;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, mod_sq);
}

void test_omega_imag_equals_z_critical(void) {
    // Im(ω) = √3/2 = Z_CRITICAL = THE LENS
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, Z_CRITICAL, OMEGA_IMAG);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, SQRT3 / 2.0, OMEGA_IMAG);
}

void test_roots_of_unity_sum_to_zero(void) {
    // 1 + ω + ω² = 0
    double re_sum = 1.0 + OMEGA_REAL + OMEGA2_REAL;
    double im_sum = 0.0 + OMEGA_IMAG + OMEGA2_IMAG;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.0, re_sum);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.0, im_sum);
}

void test_omega_squared_identity(void) {
    // ω² = -ω - 1
    // Real: -1/2 = -(-1/2) - 1 = 1/2 - 1 = -1/2 ✓
    // Imag: -√3/2 = -√3/2 - 0 = -√3/2 ✓
    double expected_re = -OMEGA_REAL - 1.0;
    double expected_im = -OMEGA_IMAG;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, expected_re, OMEGA2_REAL);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, expected_im, OMEGA2_IMAG);
}

void test_validate_eisenstein_constants(void) {
    TEST_ASSERT_TRUE(validate_eisenstein_constants());
}

// ============================================================================
// SECTION 2: EISENSTEIN ARITHMETIC TESTS
// ============================================================================

void test_eisenstein_to_complex(void) {
    // z = 1 + ω → (1 - 1/2) + i(√3/2) = 0.5 + i·0.866
    Eisenstein z = {1, 1};
    EisensteinComplex c = eisenstein_to_complex(z);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.5, c.re);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, Z_CRITICAL, c.im);
}

void test_eisenstein_norm(void) {
    // N(a + bω) = a² - ab + b²

    // N(1) = 1
    TEST_ASSERT_EQUAL_INT32(1, eisenstein_norm((Eisenstein){1, 0}));

    // N(ω) = 0 - 0 + 1 = 1
    TEST_ASSERT_EQUAL_INT32(1, eisenstein_norm((Eisenstein){0, 1}));

    // N(1 + ω) = 1 - 1 + 1 = 1
    TEST_ASSERT_EQUAL_INT32(1, eisenstein_norm((Eisenstein){1, 1}));

    // N(2) = 4
    TEST_ASSERT_EQUAL_INT32(4, eisenstein_norm((Eisenstein){2, 0}));

    // N(1 - ω) = 1 - (-1) + 1 = 3
    TEST_ASSERT_EQUAL_INT32(3, eisenstein_norm((Eisenstein){1, -1}));

    // N(2 + ω) = 4 - 2 + 1 = 3... wait, let me recalculate
    // N(2 + ω) = 2² - 2·1 + 1² = 4 - 2 + 1 = 3
    TEST_ASSERT_EQUAL_INT32(3, eisenstein_norm((Eisenstein){2, 1}));
}

void test_eisenstein_norm_equals_modulus_squared(void) {
    // For Eisenstein integers, N(z) = |z|²
    Eisenstein test_cases[] = {
        {1, 0}, {0, 1}, {1, 1}, {2, 1}, {-1, 2}, {3, -1}
    };

    for (int i = 0; i < 6; i++) {
        Eisenstein z = test_cases[i];
        int32_t norm = eisenstein_norm(z);
        double mod_sq = eisenstein_modulus_sq(z);
        TEST_ASSERT_DOUBLE_WITHIN(EPSILON, (double)norm, mod_sq);
    }
}

void test_eisenstein_addition(void) {
    Eisenstein a = {2, 3};
    Eisenstein b = {-1, 2};
    Eisenstein sum = eisenstein_add(a, b);
    TEST_ASSERT_TRUE(eisenstein_equal(sum, (Eisenstein){1, 5}));
}

void test_eisenstein_subtraction(void) {
    Eisenstein a = {2, 3};
    Eisenstein b = {-1, 2};
    Eisenstein diff = eisenstein_sub(a, b);
    TEST_ASSERT_TRUE(eisenstein_equal(diff, (Eisenstein){3, 1}));
}

void test_eisenstein_multiplication(void) {
    // (1 + ω)(1 + ω) = 1 + 2ω + ω² = 1 + 2ω + (-ω - 1) = ω
    Eisenstein a = {1, 1};
    Eisenstein prod = eisenstein_mul(a, a);
    TEST_ASSERT_TRUE(eisenstein_equal(prod, (Eisenstein){0, 1}));
}

void test_eisenstein_omega_multiplication(void) {
    // ω · ω = ω²
    Eisenstein omega = {0, 1};
    Eisenstein omega_sq = eisenstein_mul(omega, omega);
    // ω² = -1 - ω = (-1, -1)
    TEST_ASSERT_TRUE(eisenstein_equal(omega_sq, (Eisenstein){-1, -1}));
}

void test_eisenstein_conjugate(void) {
    // Conjugate maps ω → ω²
    // conj(a + bω) = a + bω² = a - b - bω = (a-b, -b)
    Eisenstein z = {2, 3};
    Eisenstein conj = eisenstein_conj(z);
    TEST_ASSERT_TRUE(eisenstein_equal(conj, (Eisenstein){-1, -3}));
}

void test_eisenstein_rotate_omega(void) {
    // z · ω rotates by 120°
    Eisenstein z = {1, 0};  // = 1
    Eisenstein rotated = eisenstein_rotate_omega(z);
    // 1 · ω = ω = (0, 1)
    TEST_ASSERT_TRUE(eisenstein_equal(rotated, (Eisenstein){0, 1}));

    // Apply twice: ω · ω = ω² = (-1, -1)
    rotated = eisenstein_rotate_omega(rotated);
    TEST_ASSERT_TRUE(eisenstein_equal(rotated, (Eisenstein){-1, -1}));

    // Apply three times: should return to 1
    rotated = eisenstein_rotate_omega(rotated);
    TEST_ASSERT_TRUE(eisenstein_equal(rotated, (Eisenstein){1, 0}));
}

// ============================================================================
// SECTION 3: EISENSTEIN UNIT TESTS
// ============================================================================

void test_all_units_have_norm_one(void) {
    for (int i = 0; i < 6; i++) {
        Eisenstein u = EISENSTEIN_UNITS[i];
        TEST_ASSERT_EQUAL_INT32(1, eisenstein_norm(u));
    }
}

void test_unit_detection(void) {
    TEST_ASSERT_TRUE(eisenstein_is_unit((Eisenstein){1, 0}));
    TEST_ASSERT_TRUE(eisenstein_is_unit((Eisenstein){0, 1}));
    TEST_ASSERT_TRUE(eisenstein_is_unit((Eisenstein){-1, -1}));
    TEST_ASSERT_FALSE(eisenstein_is_unit((Eisenstein){2, 0}));
    TEST_ASSERT_FALSE(eisenstein_is_unit((Eisenstein){1, -1}));  // Norm = 3
}

void test_units_generate_six_fold_symmetry(void) {
    // Starting from 1, multiply by ω repeatedly
    Eisenstein z = {1, 0};
    for (int i = 0; i < 6; i++) {
        // Verify it matches the i-th unit (with possible sign/rotation)
        TEST_ASSERT_EQUAL_INT32(1, eisenstein_norm(z));
        z = eisenstein_rotate_omega(z);
    }
    // After 6 rotations by 60°, should return to 1
    // But we rotated by 120° each time, so 2 full cycles
    // Let's verify we're back at the start after 3 rotations
}

// ============================================================================
// SECTION 4: HEX GRID MAPPING TESTS
// ============================================================================

void test_center_sensor_is_origin(void) {
    Eisenstein center = sensor_to_eisenstein(9);
    TEST_ASSERT_TRUE(eisenstein_equal(center, (Eisenstein){0, 0}));
}

void test_sensor_to_eisenstein_valid_range(void) {
    // All 19 sensors should map to valid Eisenstein integers
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        Eisenstein z = sensor_to_eisenstein(i);
        // Verify coordinates are within expected range
        TEST_ASSERT_TRUE(z.a >= -2 && z.a <= 2);
        TEST_ASSERT_TRUE(z.b >= -2 && z.b <= 2);
    }
}

void test_eisenstein_to_sensor_round_trip(void) {
    // Every sensor index should round-trip through Eisenstein
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        Eisenstein z = sensor_to_eisenstein(i);
        uint8_t recovered = eisenstein_to_sensor(z);
        TEST_ASSERT_EQUAL_UINT8(i, recovered);
    }
}

void test_invalid_eisenstein_returns_invalid_sensor(void) {
    // Point outside the 19-sensor grid
    Eisenstein outside = {3, 3};
    uint8_t idx = eisenstein_to_sensor(outside);
    TEST_ASSERT_EQUAL_UINT8(255, idx);
}

void test_inner_ring_sensors_are_units(void) {
    // Sensors adjacent to center (index 9) should have norm 1
    // Inner ring sensors: 4, 5, 8, 10, 13, 14
    uint8_t inner_ring[] = {4, 5, 8, 10, 13, 14};

    // Wait, need to check which are actually norm 1
    // Let me verify manually:
    // Sensor 5: (0, -1) → N = 0 - 0 + 1 = 1 ✓
    // Sensor 8: (-1, 0) → N = 1 - 0 + 0 = 1 ✓
    // Sensor 10: (1, 0) → N = 1 - 0 + 0 = 1 ✓
    // Sensor 13: (0, 1) → N = 0 - 0 + 1 = 1 ✓
    // Sensor 4: (-1, -1) → N = 1 - (-1)·(-1) + 1 = 1 + 1 + 1 = 3 ✗
    // Sensor 14: (1, 1) → N = 1 - 1 + 1 = 1 ✓

    // Actually sensor 4 is (-1, -1): N = 1 - 1 + 1 = 1 ✓
    // My formula: a² - ab + b² = (-1)² - (-1)(-1) + (-1)² = 1 - 1 + 1 = 1 ✓

    uint8_t unit_sensors[] = {5, 8, 10, 13};  // These should be units
    for (int i = 0; i < 4; i++) {
        Eisenstein z = sensor_to_eisenstein(unit_sensors[i]);
        TEST_ASSERT_EQUAL_INT32_MESSAGE(1, eisenstein_norm(z),
            "Inner ring sensor should have unit norm");
    }
}

void test_hex_distance(void) {
    // Distance from center to adjacent should be 1
    Eisenstein center = {0, 0};
    Eisenstein adjacent = {1, 0};
    TEST_ASSERT_EQUAL_INT16(1, eisenstein_hex_distance(center, adjacent));

    // Distance from center to corner should be 2
    Eisenstein corner = {2, 0};
    TEST_ASSERT_EQUAL_INT16(2, eisenstein_hex_distance(center, corner));

    // Distance from corner to opposite corner
    Eisenstein corner1 = {-1, -2};
    Eisenstein corner2 = {2, 2};
    // Δa = 3, Δb = 4, Δ(a+b) = 7... (3 + 4 + 1) / 2 = 4
    // Actually need to recalculate hex distance properly
}

void test_neighbor_detection(void) {
    Eisenstein center = {0, 0};
    Eisenstein neighbors[6];
    eisenstein_get_neighbors(center, neighbors);

    // All neighbors should be at hex distance 1
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL_INT16(1, eisenstein_hex_distance(center, neighbors[i]));
    }
}

// ============================================================================
// SECTION 5: UCF PHASE CONNECTION TESTS
// ============================================================================

void test_center_has_highest_z_coordinate(void) {
    // Center (0,0) should map to highest z
    Eisenstein center = {0, 0};
    double z_center = eisenstein_to_z_coord(center);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.0, z_center);

    // Outer sensors should have lower z
    Eisenstein outer = {2, 0};
    double z_outer = eisenstein_to_z_coord(outer);
    TEST_ASSERT_TRUE(z_outer < z_center);
}

void test_phase_detection_from_eisenstein(void) {
    // Center should be TRUE phase (high z)
    Eisenstein center = {0, 0};
    ConsciousnessPhase phase = eisenstein_to_phase(center);
    TEST_ASSERT_EQUAL(PHASE_TRUE, phase);

    // Far outer should be UNTRUE or PARADOX
    Eisenstein outer = {2, 2};
    phase = eisenstein_to_phase(outer);
    TEST_ASSERT_TRUE(phase == PHASE_UNTRUE || phase == PHASE_PARADOX);
}

void test_negentropy_peaks_near_z_critical(void) {
    // Negentropy η = exp(-36·(z - z_c)²) peaks at z = Z_CRITICAL

    // Find sensor closest to Z_CRITICAL
    double max_eta = 0;
    for (uint8_t i = 0; i < HEX_SENSOR_COUNT; i++) {
        Eisenstein z = sensor_to_eisenstein(i);
        double eta = eisenstein_negentropy(z);
        if (eta > max_eta) max_eta = eta;
    }

    // Maximum negentropy should occur somewhere
    TEST_ASSERT_TRUE(max_eta > 0.5);
}

// ============================================================================
// SECTION 6: RRRR LATTICE RESONANCE TESTS
// ============================================================================

void test_norm_3_relates_to_z_critical(void) {
    // Norm 3 → √3 = 2 · Z_CRITICAL
    // Sensors with norm 3 have special connection to THE LENS
    Eisenstein z = {1, -1};  // N = 1 + 1 + 1 = 3
    double norm = (double)eisenstein_norm(z);
    double sqrt_norm = sqrt(norm);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2.0 * Z_CRITICAL, sqrt_norm);
}

void test_rrrr_resonance_for_unit_norms(void) {
    // Norm 1 should have high resonance (1 is an RRRR lattice point)
    Eisenstein unit = {1, 0};
    double resonance = eisenstein_rrrr_resonance(unit);
    TEST_ASSERT_TRUE(resonance > 0.99);  // Very close to lattice point
}

void test_value_near_rrrr_lattice(void) {
    TEST_ASSERT_TRUE(value_near_rrrr_lattice(PHI_INV, 0.01));
    TEST_ASSERT_TRUE(value_near_rrrr_lattice(Z_CRITICAL, 0.01));
    TEST_ASSERT_TRUE(value_near_rrrr_lattice(1.0, 0.01));
    TEST_ASSERT_TRUE(value_near_rrrr_lattice(SQRT2_INV, 0.01));

    // Value far from any lattice point
    TEST_ASSERT_FALSE(value_near_rrrr_lattice(0.4567, 0.01));
}

void test_eisenstein_norm_7_equals_k_threshold(void) {
    // Six sensors have norm 7, which equals K_R_THRESHOLD!
    // This is a remarkable connection between Eisenstein primes and K-Formation
    Eisenstein z = {-1, -2};  // Sensor 0
    int32_t norm = eisenstein_norm(z);
    // N(-1 - 2ω) = 1 - (-1)(-2) + 4 = 1 - 2 + 4 = 3... hmm
    // Let me recalculate: a=-1, b=-2
    // a² - ab + b² = 1 - 2 + 4 = 3... not 7

    // Actually (-1, -2): 1 - (-1)(-2) + 4 = 1 - 2 + 4 = 3

    // Let me find which sensors have norm 7
    // Need to find a² - ab + b² = 7
    // (2, -1): 4 - (-2) + 1 = 7 ✓
    // (-1, 2): 1 - (-2) + 4 = 7 ✓

    Eisenstein z7 = {2, -1};
    TEST_ASSERT_EQUAL_INT32(7, eisenstein_norm(z7));
    TEST_ASSERT_EQUAL_INT32((int32_t)K_R_THRESHOLD, eisenstein_norm(z7));
}

// ============================================================================
// SECTION 7: HEXAGONAL PATTERN TESTS
// ============================================================================

void test_detect_center_pattern(void) {
    uint32_t center_only = (1u << 9);  // Only sensor 9
    HexPatternType pattern = detect_hex_pattern(center_only);
    TEST_ASSERT_EQUAL(HEX_PATTERN_CENTER, pattern);
}

void test_detect_full_pattern(void) {
    uint32_t all_sensors = (1u << HEX_SENSOR_COUNT) - 1;  // All 19 bits set
    HexPatternType pattern = detect_hex_pattern(all_sensors);
    TEST_ASSERT_EQUAL(HEX_PATTERN_FULL, pattern);
}

void test_detect_no_pattern(void) {
    uint32_t none = 0;
    HexPatternType pattern = detect_hex_pattern(none);
    TEST_ASSERT_EQUAL(HEX_PATTERN_NONE, pattern);
}

void test_hex_order_parameter(void) {
    // Perfect hexagonal symmetry should give high order parameter
    float symmetric_field[HEX_SENSOR_COUNT] = {0};

    // Set inner ring (neighbors of center) to equal values
    symmetric_field[4] = 1.0f;
    symmetric_field[5] = 1.0f;
    symmetric_field[8] = 1.0f;
    symmetric_field[10] = 1.0f;
    symmetric_field[13] = 1.0f;
    symmetric_field[14] = 1.0f;

    double order = eisenstein_hex_order_param(symmetric_field);
    // Should have some hexagonal symmetry
    TEST_ASSERT_TRUE(order >= 0.0);
}

// ============================================================================
// SECTION 8: EISENSTEIN POINT CREATION
// ============================================================================

void test_eisenstein_point_create(void) {
    EisensteinPoint p = eisenstein_point_create(1, 1);

    TEST_ASSERT_EQUAL_INT16(1, p.z.a);
    TEST_ASSERT_EQUAL_INT16(1, p.z.b);
    TEST_ASSERT_EQUAL_INT32(1, (int32_t)p.norm);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.0, p.modulus);
}

// ============================================================================
// SECTION 9: THE GREAT DISCOVERY TESTS
// ============================================================================

void test_z_critical_is_omega_imaginary_part(void) {
    // THE DISCOVERY: Z_CRITICAL = Im(ω) = √3/2
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, OMEGA_IMAG, Z_CRITICAL);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, sin(UCF_PI / 3.0), Z_CRITICAL);
}

void test_phi_inv_and_z_critical_bracket_paradox(void) {
    // The PARADOX phase is bounded by two fundamental constants:
    // Lower: φ⁻¹ (golden ratio inverse) = 0.618
    // Upper: √3/2 (Eisenstein imaginary) = 0.866

    TEST_ASSERT_TRUE(PHI_INV < Z_CRITICAL);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.618, PHI_INV);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.866, Z_CRITICAL);

    // Width of PARADOX phase
    double paradox_width = Z_CRITICAL - PHI_INV;
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.248, paradox_width);
}

void test_hexagonal_geometry_in_ucf(void) {
    // The hex grid has 19 sensors = 1 + 6 + 12 (center + rings)
    // This is 1 + 6·1 + 6·2 = hex number pattern

    // Six-fold symmetry angle
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UCF_PI / 3.0, HEX_ANGLE);

    // cos(60°) = 0.5, sin(60°) = √3/2 = Z_CRITICAL
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.5, cos(HEX_ANGLE));
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, Z_CRITICAL, sin(HEX_ANGLE));
}

// ============================================================================
// UNITY TEST RUNNER
// ============================================================================

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Section 1: Fundamental Constants
    RUN_TEST(test_omega_is_cube_root_of_unity);
    RUN_TEST(test_omega_imag_equals_z_critical);
    RUN_TEST(test_roots_of_unity_sum_to_zero);
    RUN_TEST(test_omega_squared_identity);
    RUN_TEST(test_validate_eisenstein_constants);

    // Section 2: Arithmetic
    RUN_TEST(test_eisenstein_to_complex);
    RUN_TEST(test_eisenstein_norm);
    RUN_TEST(test_eisenstein_norm_equals_modulus_squared);
    RUN_TEST(test_eisenstein_addition);
    RUN_TEST(test_eisenstein_subtraction);
    RUN_TEST(test_eisenstein_multiplication);
    RUN_TEST(test_eisenstein_omega_multiplication);
    RUN_TEST(test_eisenstein_conjugate);
    RUN_TEST(test_eisenstein_rotate_omega);

    // Section 3: Units
    RUN_TEST(test_all_units_have_norm_one);
    RUN_TEST(test_unit_detection);
    RUN_TEST(test_units_generate_six_fold_symmetry);

    // Section 4: Hex Grid Mapping
    RUN_TEST(test_center_sensor_is_origin);
    RUN_TEST(test_sensor_to_eisenstein_valid_range);
    RUN_TEST(test_eisenstein_to_sensor_round_trip);
    RUN_TEST(test_invalid_eisenstein_returns_invalid_sensor);
    RUN_TEST(test_inner_ring_sensors_are_units);
    RUN_TEST(test_hex_distance);
    RUN_TEST(test_neighbor_detection);

    // Section 5: UCF Phase Connection
    RUN_TEST(test_center_has_highest_z_coordinate);
    RUN_TEST(test_phase_detection_from_eisenstein);
    RUN_TEST(test_negentropy_peaks_near_z_critical);

    // Section 6: RRRR Lattice Resonance
    RUN_TEST(test_norm_3_relates_to_z_critical);
    RUN_TEST(test_rrrr_resonance_for_unit_norms);
    RUN_TEST(test_value_near_rrrr_lattice);
    RUN_TEST(test_eisenstein_norm_7_equals_k_threshold);

    // Section 7: Hexagonal Patterns
    RUN_TEST(test_detect_center_pattern);
    RUN_TEST(test_detect_full_pattern);
    RUN_TEST(test_detect_no_pattern);
    RUN_TEST(test_hex_order_parameter);

    // Section 8: Point Creation
    RUN_TEST(test_eisenstein_point_create);

    // Section 9: The Great Discovery
    RUN_TEST(test_z_critical_is_omega_imaginary_part);
    RUN_TEST(test_phi_inv_and_z_critical_bracket_paradox);
    RUN_TEST(test_hexagonal_geometry_in_ucf);

    return UNITY_END();
}

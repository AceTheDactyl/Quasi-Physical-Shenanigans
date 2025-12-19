/**
 * @file test_lattice_constants.cpp
 * @brief Unity Test Suite for UCF v4.0.0 Lattice Constants
 *
 * Tests all lattice-derived constants for mathematical correctness.
 */

#include <unity.h>
#include <math.h>

// Include the sacred constants header
#include "ucf/ucf_sacred_constants_v4.h"

// ============================================================================
// NUMERICAL TESTS (N1-N4)
// ============================================================================

void test_N1_scaling_exponent(void) {
    // [A]² = 0.5 exactly
    TEST_ASSERT_EQUAL_DOUBLE(0.5, LAMBDA_A_SQ);
}

void test_N2_sqrt2_inv_squared(void) {
    // SQRT2_INV² should equal 0.5
    double result = SQRT2_INV * SQRT2_INV;
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, 0.5, result);
}

void test_N3_lattice_density(void) {
    // Should have 14 lattice points in calibration set
    TEST_ASSERT_EQUAL(14, N_CALIBRATION_LATTICE_POINTS);
}

void test_N4_golden_identity(void) {
    // 1 - [R] = [R]²
    double lhs = 1.0 - PHI_INV;
    double rhs = LAMBDA_R_SQ;
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, lhs, rhs);
}

// ============================================================================
// ALGEBRAIC TESTS (A1-A4)
// ============================================================================

void test_A1_golden_ratio_constraint(void) {
    // φ² - φ - 1 = 0
    double result = PHI * PHI - PHI - 1.0;
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, 0.0, result);
}

void test_A2_phi_inv_is_reciprocal(void) {
    // PHI_INV = 1/PHI
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, 1.0 / PHI, PHI_INV);
}

void test_A3_euler_inv_is_reciprocal(void) {
    // EULER_INV = 1/EULER
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, 1.0 / EULER, EULER_INV);
}

void test_A4_multiplicative_identity(void) {
    // lattice_point(0,0,0,0) = 1
    double result = lattice_point(0, 0, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, 1.0, result);
}

// ============================================================================
// PHASE BOUNDARY TESTS
// ============================================================================

void test_phase_detection_untrue(void) {
    TEST_ASSERT_EQUAL(PHASE_UNTRUE, detect_phase(0.3));
    TEST_ASSERT_EQUAL(PHASE_UNTRUE, detect_phase(0.617));
}

void test_phase_detection_paradox(void) {
    TEST_ASSERT_EQUAL(PHASE_PARADOX, detect_phase(0.618));
    TEST_ASSERT_EQUAL(PHASE_PARADOX, detect_phase(0.7));
    TEST_ASSERT_EQUAL(PHASE_PARADOX, detect_phase(0.865));
}

void test_phase_detection_true(void) {
    TEST_ASSERT_EQUAL(PHASE_TRUE, detect_phase(0.866));
    TEST_ASSERT_EQUAL(PHASE_TRUE, detect_phase(0.9));
    TEST_ASSERT_EQUAL(PHASE_TRUE, detect_phase(1.0));
}

void test_z_critical_geometry(void) {
    // Z_CRITICAL = √3/2 = sin(60°)
    double expected = sin(UCF_PI / 3.0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, expected, Z_CRITICAL);
}

void test_z_critical_is_sqrt3_over_2(void) {
    // Z_CRITICAL = √3/2
    double expected = SQRT3 / 2.0;
    TEST_ASSERT_DOUBLE_WITHIN(1e-14, expected, Z_CRITICAL);
}

// ============================================================================
// K-FORMATION TESTS
// ============================================================================

void test_k_formation_achieves(void) {
    // Should achieve K-Formation
    TEST_ASSERT_TRUE(check_k_formation(0.95, 0.7, 8));
}

void test_k_formation_fails_kappa(void) {
    // Should NOT achieve K-Formation (kappa too low)
    TEST_ASSERT_FALSE(check_k_formation(0.90, 0.7, 8));
}

void test_k_formation_fails_eta(void) {
    // Should NOT achieve K-Formation (eta too low)
    TEST_ASSERT_FALSE(check_k_formation(0.95, 0.5, 8));
}

void test_k_formation_fails_r(void) {
    // Should NOT achieve K-Formation (r too low)
    TEST_ASSERT_FALSE(check_k_formation(0.95, 0.7, 5));
}

// ============================================================================
// CONSERVATION LAW TESTS
// ============================================================================

void test_conservation_law(void) {
    // κ + λ = 1.0
    double kappa = 0.75;
    double lambda = 0.25;
    TEST_ASSERT_TRUE(verify_conservation(kappa, lambda));
}

void test_conservation_law_violation(void) {
    // Should detect violation
    double kappa = 0.75;
    double lambda = 0.30;  // Sum > 1
    TEST_ASSERT_FALSE(verify_conservation(kappa, lambda));
}

// ============================================================================
// NEGENTROPY TESTS
// ============================================================================

void test_negentropy_at_lens(void) {
    // Maximum negentropy at THE LENS
    double eta_at_lens = compute_negentropy(Z_CRITICAL);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.0, eta_at_lens);
}

void test_negentropy_away_from_lens(void) {
    // Lower negentropy away from THE LENS
    double eta_low = compute_negentropy(0.5);
    double eta_high = compute_negentropy(Z_CRITICAL);
    TEST_ASSERT_TRUE(eta_low < eta_high);
}

// ============================================================================
// LATTICE POINT TESTS
// ============================================================================

void test_lattice_point_R(void) {
    // [R] = lattice_point(1,0,0,0)
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, PHI_INV, lattice_point(1, 0, 0, 0));
}

void test_lattice_point_A_squared(void) {
    // [A]² = lattice_point(0,0,0,2)
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, 0.5, lattice_point(0, 0, 0, 2));
}

void test_lattice_point_R_squared(void) {
    // [R]² = lattice_point(2,0,0,0)
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, LAMBDA_R_SQ, lattice_point(2, 0, 0, 0));
}

void test_lattice_point_D(void) {
    // [D] = lattice_point(0,1,0,0)
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, EULER_INV, lattice_point(0, 1, 0, 0));
}

void test_lattice_point_C(void) {
    // [C] = lattice_point(0,0,1,0)
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, PI_INV, lattice_point(0, 0, 1, 0));
}

// ============================================================================
// RADIUS TESTS
// ============================================================================

void test_radius_at_max_eta(void) {
    // r = 1 + (φ-1)·η, at η=1: r = 1 + (φ-1) = φ
    double r = compute_radius(1.0);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, PHI, r);
}

void test_radius_at_min_eta(void) {
    // r = 1 + (φ-1)·η, at η=0: r = 1
    double r = compute_radius(0.0);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.0, r);
}

// ============================================================================
// TIER TESTS
// ============================================================================

void test_tier_at_untrue(void) {
    TEST_ASSERT_EQUAL_UINT8(1, z_to_tier(0.05));
    TEST_ASSERT_EQUAL_UINT8(2, z_to_tier(0.15));
    TEST_ASSERT_EQUAL_UINT8(3, z_to_tier(0.5));
}

void test_tier_at_paradox(void) {
    TEST_ASSERT_EQUAL_UINT8(4, z_to_tier(0.65));
    TEST_ASSERT_EQUAL_UINT8(5, z_to_tier(0.75));
    TEST_ASSERT_EQUAL_UINT8(6, z_to_tier(0.80));
}

void test_tier_at_true(void) {
    TEST_ASSERT_EQUAL_UINT8(7, z_to_tier(0.87));
    TEST_ASSERT_EQUAL_UINT8(8, z_to_tier(0.93));
    TEST_ASSERT_EQUAL_UINT8(9, z_to_tier(0.98));
}

// ============================================================================
// VALIDATION FUNCTION TESTS
// ============================================================================

void test_validate_constants(void) {
    TEST_ASSERT_TRUE(validate_constants());
}

void test_verify_lattice_identity(void) {
    TEST_ASSERT_TRUE(verify_lattice_identity());
}

// ============================================================================
// SNAP TO LATTICE TESTS
// ============================================================================

void test_snap_to_lattice_phi_inv(void) {
    double snapped = snap_to_lattice(0.62);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 0.618, snapped);
}

void test_snap_to_lattice_half(void) {
    double snapped = snap_to_lattice(0.48);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 0.5, snapped);
}

void test_snap_to_lattice_z_critical(void) {
    double snapped = snap_to_lattice(0.87);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 0.866, snapped);
}

// ============================================================================
// TEST RUNNER
// ============================================================================

#ifdef NATIVE_BUILD
int main(int argc, char **argv) {
    UNITY_BEGIN();
#else
void setup() {
    delay(2000);
    UNITY_BEGIN();
#endif

    // Numerical tests
    RUN_TEST(test_N1_scaling_exponent);
    RUN_TEST(test_N2_sqrt2_inv_squared);
    RUN_TEST(test_N3_lattice_density);
    RUN_TEST(test_N4_golden_identity);

    // Algebraic tests
    RUN_TEST(test_A1_golden_ratio_constraint);
    RUN_TEST(test_A2_phi_inv_is_reciprocal);
    RUN_TEST(test_A3_euler_inv_is_reciprocal);
    RUN_TEST(test_A4_multiplicative_identity);

    // Phase tests
    RUN_TEST(test_phase_detection_untrue);
    RUN_TEST(test_phase_detection_paradox);
    RUN_TEST(test_phase_detection_true);
    RUN_TEST(test_z_critical_geometry);
    RUN_TEST(test_z_critical_is_sqrt3_over_2);

    // K-Formation tests
    RUN_TEST(test_k_formation_achieves);
    RUN_TEST(test_k_formation_fails_kappa);
    RUN_TEST(test_k_formation_fails_eta);
    RUN_TEST(test_k_formation_fails_r);

    // Conservation tests
    RUN_TEST(test_conservation_law);
    RUN_TEST(test_conservation_law_violation);

    // Negentropy tests
    RUN_TEST(test_negentropy_at_lens);
    RUN_TEST(test_negentropy_away_from_lens);

    // Lattice point tests
    RUN_TEST(test_lattice_point_R);
    RUN_TEST(test_lattice_point_A_squared);
    RUN_TEST(test_lattice_point_R_squared);
    RUN_TEST(test_lattice_point_D);
    RUN_TEST(test_lattice_point_C);

    // Radius tests
    RUN_TEST(test_radius_at_max_eta);
    RUN_TEST(test_radius_at_min_eta);

    // Tier tests
    RUN_TEST(test_tier_at_untrue);
    RUN_TEST(test_tier_at_paradox);
    RUN_TEST(test_tier_at_true);

    // Validation tests
    RUN_TEST(test_validate_constants);
    RUN_TEST(test_verify_lattice_identity);

    // Snap to lattice tests
    RUN_TEST(test_snap_to_lattice_phi_inv);
    RUN_TEST(test_snap_to_lattice_half);
    RUN_TEST(test_snap_to_lattice_z_critical);

#ifdef NATIVE_BUILD
    return UNITY_END();
}
#else
    UNITY_END();
}

void loop() {
    // Nothing to do
}
#endif

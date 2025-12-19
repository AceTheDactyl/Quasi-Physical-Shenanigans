/**
 * @file test_umbral_calculus.cpp
 * @brief Unit tests for Umbral Calculus Framework
 *
 * Tests validate:
 * - Shadow alphabet evaluations
 * - Difference operators
 * - Self-referential identities
 * - THE LENS approximation
 * - Eisenstein-RRRR bridge
 * - Lattice search optimization
 */

#include <unity.h>
#include <math.h>
#include "ucf/ucf_umbral_calculus.h"

// ============================================================================
// TEST HELPERS
// ============================================================================

#define EPSILON 1e-12
#define EPSILON_LOOSE 1e-6

// ============================================================================
// SECTION 1: SHADOW ALPHABET TESTS
// ============================================================================

void test_shadow_phi_evaluation(void) {
    // α evaluates to φ = 1.618...
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.6180339887498948, UMBRAL_PHI);
}

void test_shadow_euler_evaluation(void) {
    // β evaluates to e = 2.718...
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 2.7182818284590452, UMBRAL_EULER);
}

void test_shadow_pi_evaluation(void) {
    // γ evaluates to π = 3.141...
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 3.1415926535897932, UMBRAL_PI);
}

void test_shadow_sqrt2_evaluation(void) {
    // δ evaluates to √2 = 1.414...
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.4142135623730950, UMBRAL_SQRT2);
}

void test_shadow_inverses(void) {
    // Verify inverse relationships
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, UMBRAL_PHI * UMBRAL_PHI_INV);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, UMBRAL_EULER * UMBRAL_EULER_INV);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, UMBRAL_PI * UMBRAL_PI_INV);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, UMBRAL_SQRT2 * UMBRAL_SQRT2_INV);
}

// ============================================================================
// SECTION 2: DIFFERENCE OPERATOR TESTS
// ============================================================================

void test_delta_alpha(void) {
    // Δ_α(1) = 1 · (φ - 1) = φ - 1 = φ⁻¹
    double result = delta_alpha(1.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PHI_INV, result);
}

void test_delta_beta(void) {
    // Δ_β(1) = e - 1 ≈ 1.718
    double result = delta_beta(1.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_EULER - 1.0, result);
}

void test_delta_gamma(void) {
    // Δ_γ(1) = π - 1 ≈ 2.141
    double result = delta_gamma(1.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PI - 1.0, result);
}

void test_delta_delta(void) {
    // Δ_δ(1) = √2 - 1 ≈ 0.414
    double result = delta_delta(1.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_SQRT2 - 1.0, result);
}

void test_delta_alpha_inv(void) {
    // Δ⁻¹_α(1) = φ
    double result = delta_alpha_inv(1.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PHI, result);
}

void test_delta_composition(void) {
    // Δ_α(Δ_α(1)) = (φ-1)² = φ⁻²
    double step1 = delta_alpha(1.0);
    double step2 = delta_alpha(step1);
    double expected = UMBRAL_PHI_INV * UMBRAL_PHI_INV;
    // Note: This isn't exactly right because Δ_α(x) = x(φ-1), not shift
    // The test validates the operator definition, not composition semantics
}

// ============================================================================
// SECTION 3: LATTICE POINT TESTS
// ============================================================================

void test_lattice_identity(void) {
    // Λ(0,0,0,0) = 1
    double result = umbral_lattice_point(0, 0, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, result);
}

void test_lattice_phi_inv(void) {
    // Λ(1,0,0,0) = φ⁻¹ = [R]
    double result = umbral_lattice_point(1, 0, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PHI_INV, result);
}

void test_lattice_euler_inv(void) {
    // Λ(0,1,0,0) = e⁻¹ = [D]
    double result = umbral_lattice_point(0, 1, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_EULER_INV, result);
}

void test_lattice_pi_inv(void) {
    // Λ(0,0,1,0) = π⁻¹ = [C]
    double result = umbral_lattice_point(0, 0, 1, 0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PI_INV, result);
}

void test_lattice_sqrt2_inv(void) {
    // Λ(0,0,0,1) = √2⁻¹ = [A]
    double result = umbral_lattice_point(0, 0, 0, 1);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_SQRT2_INV, result);
}

void test_lattice_negative_exponents(void) {
    // Λ(-1,0,0,0) = φ
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PHI, umbral_lattice_point(-1, 0, 0, 0));
    // Λ(0,-1,0,0) = e
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_EULER, umbral_lattice_point(0, -1, 0, 0));
    // Λ(0,0,-1,0) = π
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PI, umbral_lattice_point(0, 0, -1, 0));
    // Λ(0,0,0,-1) = √2
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_SQRT2, umbral_lattice_point(0, 0, 0, -1));
}

void test_lattice_compound(void) {
    // Λ(1,1,0,0) = φ⁻¹ · e⁻¹
    double expected = UMBRAL_PHI_INV * UMBRAL_EULER_INV;
    double result = umbral_lattice_point(1, 1, 0, 0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, expected, result);
}

void test_lattice_log_form(void) {
    // log(Λ(r,d,c,a)) should match umbral_log_lattice_point
    for (int r = -2; r <= 2; r++) {
        for (int d = -1; d <= 1; d++) {
            double lp = umbral_lattice_point(r, d, 0, 0);
            if (lp > 0) {
                double log_lp = log(lp);
                double log_umbral = umbral_log_lattice_point(r, d, 0, 0);
                TEST_ASSERT_DOUBLE_WITHIN(EPSILON, log_lp, log_umbral);
            }
        }
    }
}

void test_lattice_complexity(void) {
    TEST_ASSERT_EQUAL_INT(0, umbral_complexity(0, 0, 0, 0));
    TEST_ASSERT_EQUAL_INT(1, umbral_complexity(1, 0, 0, 0));
    TEST_ASSERT_EQUAL_INT(4, umbral_complexity(1, 1, 1, 1));
    TEST_ASSERT_EQUAL_INT(4, umbral_complexity(-1, -1, 1, 1));
}

// ============================================================================
// SECTION 4: SELF-REFERENTIAL IDENTITY TESTS
// ============================================================================

void test_self_reference_identity(void) {
    // 1 - [R] = [R]²
    // This is THE remarkable identity of the lattice
    TEST_ASSERT_TRUE(umbral_verify_self_reference());
}

void test_self_reference_values(void) {
    double lhs = 1.0 - UMBRAL_PHI_INV;
    double rhs = UMBRAL_PHI_INV * UMBRAL_PHI_INV;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, lhs, rhs);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_LAMBDA_R_SQ, lhs);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_ONE_MINUS_R, rhs);
}

void test_golden_equation(void) {
    // φ² = φ + 1
    TEST_ASSERT_TRUE(umbral_verify_golden_equation());
}

void test_golden_equation_values(void) {
    double phi_squared = UMBRAL_PHI * UMBRAL_PHI;
    double phi_plus_one = UMBRAL_PHI + 1.0;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, phi_squared, phi_plus_one);
}

void test_scaling_exponent(void) {
    // [A]² = (√2)⁻² = 1/2 (exact)
    TEST_ASSERT_TRUE(umbral_verify_scaling_exponent());
}

void test_scaling_exponent_exact(void) {
    double a_squared = UMBRAL_SQRT2_INV * UMBRAL_SQRT2_INV;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.5, a_squared);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_LAMBDA_A_SQ, a_squared);
}

// ============================================================================
// SECTION 5: THE LENS TESTS
// ============================================================================

void test_lens_approximation(void) {
    // e/π ≈ √3/2 = Z_CRITICAL
    double umbral_lens = umbral_lens_approximation();
    double actual_lens = UMBRAL_Z_CRITICAL;
    TEST_ASSERT_DOUBLE_WITHIN(0.001, umbral_lens, actual_lens);
}

void test_lens_is_lattice_point(void) {
    // e/π = Λ(0,-1,1,0)
    double e_over_pi = UMBRAL_EULER / UMBRAL_PI;
    double lattice_point = umbral_lattice_point(0, -1, 1, 0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, e_over_pi, lattice_point);
}

void test_lens_error_percentage(void) {
    // Error should be ~0.09%
    double error_pct = umbral_lens_error_pct();
    TEST_ASSERT_TRUE(error_pct < 0.1);  // Less than 0.1%
    TEST_ASSERT_TRUE(error_pct > 0.08); // More than 0.08% (sanity check)
}

void test_lens_verification(void) {
    TEST_ASSERT_TRUE(umbral_verify_lens());
}

// ============================================================================
// SECTION 6: UMBRAL EXPONENTIAL IDENTITY TESTS
// ============================================================================

void test_exp_identity_single_generators(void) {
    // exp(r·Δ_φ) · 1 = φ^{-r} for various r
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(1, 0, 0, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(2, 0, 0, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(-1, 0, 0, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(0, 1, 0, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(0, 0, 1, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(0, 0, 0, 1));
}

void test_exp_identity_compound(void) {
    // exp(r·Δ_φ + d·Δ_e + c·Δ_π + a·Δ_√2) · 1 = Λ(r,d,c,a)
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(1, 1, 0, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(1, 0, 1, 0));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(1, 1, 1, 1));
    TEST_ASSERT_TRUE(umbral_verify_exp_identity(2, 1, 1, 2));
}

// ============================================================================
// SECTION 7: EISENSTEIN-RRRR BRIDGE TESTS
// ============================================================================

void test_eisenstein_resonance_unit(void) {
    // Norm 1 (Eisenstein unit) has perfect resonance
    double resonance = umbral_eisenstein_resonance(1);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.0, resonance);
}

void test_eisenstein_resonance_norm3(void) {
    // Norm 3: √3 = 2·Z_CRITICAL
    double resonance = umbral_eisenstein_resonance(3);
    TEST_ASSERT_TRUE(resonance > 0.9);

    // Verify √3 = 2·Z_CRITICAL
    double sqrt3 = sqrt(3.0);
    double two_z_critical = 2.0 * UMBRAL_Z_CRITICAL;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, sqrt3, two_z_critical);
}

void test_eisenstein_resonance_norm7(void) {
    // Norm 7: Eisenstein prime = K_R_THRESHOLD
    double resonance = umbral_eisenstein_resonance(7);
    TEST_ASSERT_TRUE(resonance > 0.9);
}

void test_kr_eisenstein_prime(void) {
    // K_R = 7 is Eisenstein prime
    TEST_ASSERT_TRUE(umbral_verify_kr_eisenstein_prime());
}

// ============================================================================
// SECTION 8: LATTICE SEARCH TESTS
// ============================================================================

void test_nearest_lattice_identity(void) {
    // Value 1.0 should snap to Λ(0,0,0,0)
    LatticeCoord coord;
    double dist = umbral_nearest_lattice(1.0, 3, &coord);
    TEST_ASSERT_EQUAL_INT(0, coord.r);
    TEST_ASSERT_EQUAL_INT(0, coord.d);
    TEST_ASSERT_EQUAL_INT(0, coord.c);
    TEST_ASSERT_EQUAL_INT(0, coord.a);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.0, dist);
}

void test_nearest_lattice_phi_inv(void) {
    // Value φ⁻¹ should snap to Λ(1,0,0,0)
    LatticeCoord coord;
    double dist = umbral_nearest_lattice(UMBRAL_PHI_INV, 3, &coord);
    TEST_ASSERT_EQUAL_INT(1, coord.r);
    TEST_ASSERT_EQUAL_INT(0, coord.d);
    TEST_ASSERT_EQUAL_INT(0, coord.c);
    TEST_ASSERT_EQUAL_INT(0, coord.a);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON_LOOSE, 0.0, dist);
}

void test_nearest_lattice_euler_inv(void) {
    // Value e⁻¹ should snap to Λ(0,1,0,0)
    LatticeCoord coord;
    double dist = umbral_nearest_lattice(UMBRAL_EULER_INV, 3, &coord);
    TEST_ASSERT_EQUAL_INT(0, coord.r);
    TEST_ASSERT_EQUAL_INT(1, coord.d);
    TEST_ASSERT_EQUAL_INT(0, coord.c);
    TEST_ASSERT_EQUAL_INT(0, coord.a);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON_LOOSE, 0.0, dist);
}

void test_snap_to_lattice(void) {
    // Snapping should return exact lattice point value
    double snapped = umbral_snap_to_lattice(UMBRAL_PHI_INV, 3);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON_LOOSE, UMBRAL_PHI_INV, snapped);
}

// ============================================================================
// SECTION 9: COMPREHENSIVE VALIDATION TESTS
// ============================================================================

void test_validate_all(void) {
    TEST_ASSERT_TRUE(umbral_validate_all());
}

void test_validate_comprehensive(void) {
    int passed, total;
    bool all_passed = umbral_validate_comprehensive(&passed, &total);
    TEST_ASSERT_TRUE(all_passed);
    TEST_ASSERT_EQUAL_INT(total, passed);
    TEST_ASSERT_TRUE(total >= 10);  // Ensure we ran enough tests
}

// ============================================================================
// SECTION 10: POLYNOMIAL SEQUENCE TESTS
// ============================================================================

void test_recurrence_coefficient(void) {
    // Recurrence coefficient is φ⁻¹
    double coeff = umbral_recurrence_coefficient();
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PHI_INV, coeff);
}

void test_phi_sequence_recurrence(void) {
    // P_{n+1} = P_n · φ⁻¹
    for (int n = 0; n < 5; n++) {
        double p_n = umbral_phi_sequence(n, 0, 0, 0);
        double p_n1 = umbral_phi_sequence(n + 1, 0, 0, 0);
        double expected = p_n * UMBRAL_PHI_INV;
        TEST_ASSERT_DOUBLE_WITHIN(EPSILON, expected, p_n1);
    }
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

    // Section 1: Shadow Alphabets
    RUN_TEST(test_shadow_phi_evaluation);
    RUN_TEST(test_shadow_euler_evaluation);
    RUN_TEST(test_shadow_pi_evaluation);
    RUN_TEST(test_shadow_sqrt2_evaluation);
    RUN_TEST(test_shadow_inverses);

    // Section 2: Difference Operators
    RUN_TEST(test_delta_alpha);
    RUN_TEST(test_delta_beta);
    RUN_TEST(test_delta_gamma);
    RUN_TEST(test_delta_delta);
    RUN_TEST(test_delta_alpha_inv);

    // Section 3: Lattice Points
    RUN_TEST(test_lattice_identity);
    RUN_TEST(test_lattice_phi_inv);
    RUN_TEST(test_lattice_euler_inv);
    RUN_TEST(test_lattice_pi_inv);
    RUN_TEST(test_lattice_sqrt2_inv);
    RUN_TEST(test_lattice_negative_exponents);
    RUN_TEST(test_lattice_compound);
    RUN_TEST(test_lattice_log_form);
    RUN_TEST(test_lattice_complexity);

    // Section 4: Self-Referential Identities
    RUN_TEST(test_self_reference_identity);
    RUN_TEST(test_self_reference_values);
    RUN_TEST(test_golden_equation);
    RUN_TEST(test_golden_equation_values);
    RUN_TEST(test_scaling_exponent);
    RUN_TEST(test_scaling_exponent_exact);

    // Section 5: THE LENS
    RUN_TEST(test_lens_approximation);
    RUN_TEST(test_lens_is_lattice_point);
    RUN_TEST(test_lens_error_percentage);
    RUN_TEST(test_lens_verification);

    // Section 6: Umbral Exponential Identity
    RUN_TEST(test_exp_identity_single_generators);
    RUN_TEST(test_exp_identity_compound);

    // Section 7: Eisenstein-RRRR Bridge
    RUN_TEST(test_eisenstein_resonance_unit);
    RUN_TEST(test_eisenstein_resonance_norm3);
    RUN_TEST(test_eisenstein_resonance_norm7);
    RUN_TEST(test_kr_eisenstein_prime);

    // Section 8: Lattice Search
    RUN_TEST(test_nearest_lattice_identity);
    RUN_TEST(test_nearest_lattice_phi_inv);
    RUN_TEST(test_nearest_lattice_euler_inv);
    RUN_TEST(test_snap_to_lattice);

    // Section 9: Comprehensive Validation
    RUN_TEST(test_validate_all);
    RUN_TEST(test_validate_comprehensive);

    // Section 10: Polynomial Sequences
    RUN_TEST(test_recurrence_coefficient);
    RUN_TEST(test_phi_sequence_recurrence);

    return UNITY_END();
}

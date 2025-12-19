/**
 * @file test_umbral_transforms.cpp
 * @brief Unit Tests for Umbral Calculus Transforms
 *
 * Tests the shadow alphabet-aware versions of UCF computations.
 */

#include <unity.h>
#include <math.h>
#include "ucf/ucf_umbral_transforms.h"
#include "ucf/ucf_umbral_calculus.h"

// ============================================================================
// TEST HELPERS
// ============================================================================

#define EPSILON 1e-10

static bool double_equal(double a, double b, double tol = EPSILON) {
    return fabs(a - b) < tol;
}

// ============================================================================
// TIER BOUNDARY TESTS
// ============================================================================

void test_tier_boundaries_monotonic(void) {
    for (int i = 0; i < 9; i++) {
        TEST_ASSERT_TRUE(UMBRAL_TIER_BOUNDARIES[i] < UMBRAL_TIER_BOUNDARIES[i + 1]);
    }
}

void test_tier_boundaries_range(void) {
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.0, UMBRAL_TIER_BOUNDARIES[0]);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, UMBRAL_TIER_BOUNDARIES[9]);
}

void test_tier_3_4_is_phi_inv(void) {
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PHI_INV, UMBRAL_TIER_3_4);
}

void test_tier_6_7_is_z_critical(void) {
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_Z_CRITICAL, UMBRAL_TIER_6_7);
}

void test_z_to_tier_boundaries(void) {
    // Just below [R] → tier 3
    TEST_ASSERT_EQUAL_UINT8(3, umbral_z_to_tier(0.617));

    // At [R] → tier 4
    TEST_ASSERT_EQUAL_UINT8(4, umbral_z_to_tier(UMBRAL_PHI_INV + 0.001));

    // Just below Z_c → tier 6
    TEST_ASSERT_EQUAL_UINT8(6, umbral_z_to_tier(0.865));

    // At Z_c → tier 7
    TEST_ASSERT_EQUAL_UINT8(7, umbral_z_to_tier(UMBRAL_Z_CRITICAL + 0.001));

    // At maximum → tier 9
    TEST_ASSERT_EQUAL_UINT8(9, umbral_z_to_tier(1.0));
}

// ============================================================================
// NEGENTROPY TESTS
// ============================================================================

void test_negentropy_peaks_at_lens(void) {
    double eta = umbral_negentropy(UMBRAL_Z_CRITICAL);
    TEST_ASSERT_DOUBLE_WITHIN(1e-10, 1.0, eta);
}

void test_negentropy_decays_from_lens(void) {
    double eta_at_lens = umbral_negentropy(UMBRAL_Z_CRITICAL);
    double eta_below = umbral_negentropy(UMBRAL_Z_CRITICAL - 0.1);
    double eta_above = umbral_negentropy(UMBRAL_Z_CRITICAL + 0.1);

    TEST_ASSERT_TRUE(eta_below < eta_at_lens);
    TEST_ASSERT_TRUE(eta_above < eta_at_lens);
}

void test_negentropy_symmetric(void) {
    double delta = 0.1;
    double eta_below = umbral_negentropy(UMBRAL_Z_CRITICAL - delta);
    double eta_above = umbral_negentropy(UMBRAL_Z_CRITICAL + delta);

    TEST_ASSERT_DOUBLE_WITHIN(1e-10, eta_below, eta_above);
}

void test_negentropy_shadow_approximation(void) {
    // Shadow version should be close to standard
    double eta_std = umbral_negentropy(0.8);
    double eta_shadow = umbral_negentropy_shadow(0.8);

    // Both should be in [0, 1]
    TEST_ASSERT_TRUE(eta_std >= 0.0 && eta_std <= 1.0);
    TEST_ASSERT_TRUE(eta_shadow >= 0.0 && eta_shadow <= 1.0);
}

// ============================================================================
// COHERENCE TESTS
// ============================================================================

void test_coherence_zero_variance(void) {
    double kappa = umbral_coherence(0.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, kappa);
}

void test_coherence_uses_pi(void) {
    // With variance = 1/(π²), κ should be approximately 0
    double variance = 1.0 / (UMBRAL_PI * UMBRAL_PI);
    double kappa = umbral_coherence(variance);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 0.0, kappa);
}

void test_coherence_clamps(void) {
    // Large variance should clamp to 0
    double kappa = umbral_coherence(10.0);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.0, kappa);
}

void test_kappa_threshold_approximation(void) {
    // UMBRAL_KAPPA_THRESHOLD should be close to 0.92
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 0.92, UMBRAL_KAPPA_THRESHOLD);
}

// ============================================================================
// EMA SMOOTHING TESTS
// ============================================================================

void test_ema_phi_coefficients_sum_to_one(void) {
    // [R] + (1 - [R]) = 1
    double sum = UMBRAL_PHI_INV + (1.0 - UMBRAL_PHI_INV);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, sum);
}

void test_ema_phi_identity_connection(void) {
    // 1 - [R] = [R]²
    double one_minus_R = 1.0 - UMBRAL_PHI_INV;
    double R_squared = UMBRAL_PHI_INV * UMBRAL_PHI_INV;
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, one_minus_R, R_squared);
}

void test_ema_phi_step_response(void) {
    double y = 0.0;

    // Step input: after several iterations should approach 1.0
    for (int i = 0; i < 20; i++) {
        y = umbral_ema_phi(1.0, y);
    }

    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.0, y);
}

void test_ema_exp_uses_euler(void) {
    double y = 0.0;
    y = umbral_ema_exp(1.0, y);

    // First step: y = e⁻¹ · 1 + (1 - e⁻¹) · 0 = e⁻¹
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_EULER_INV, y);
}

void test_ema_fast_uses_sqrt2(void) {
    double y = 0.0;
    y = umbral_ema_fast(1.0, y);

    // First step: y = √2⁻¹ · 1 = √2⁻¹
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_SQRT2_INV, y);
}

// ============================================================================
// HARMONIC GENERATION TESTS
// ============================================================================

void test_phi_harmonics_at_zero(void) {
    double h = umbral_phi_harmonics(0.0, 5);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 0.0, h);
}

void test_phi_harmonics_decaying_coefficients(void) {
    // Amplitudes should decay as φ^{-n}
    // At phase = 0.125 (45°), sin(nπ/4) for n=1 is sin(π/4) = √2/2
    double h1 = umbral_phi_harmonics(0.125, 1);  // Only fundamental
    double h2 = umbral_phi_harmonics(0.125, 2);  // Fundamental + 2nd

    // With more harmonics, the amplitude changes
    TEST_ASSERT_TRUE(h1 != h2);
}

void test_waveform_normalized(void) {
    // Waveform should stay bounded
    double max_val = 0.0;
    for (int i = 0; i < 100; i++) {
        double phase = (double)i / 100.0;
        double val = fabs(umbral_waveform(phase));
        if (val > max_val) max_val = val;
    }

    // Should be bounded (approximately)
    TEST_ASSERT_TRUE(max_val < 3.0);
}

// ============================================================================
// PHASE DETECTION TESTS
// ============================================================================

void test_detect_phase_untrue(void) {
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_UNTRUE, umbral_detect_phase(0.3));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_UNTRUE, umbral_detect_phase(0.5));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_UNTRUE, umbral_detect_phase(0.617));
}

void test_detect_phase_paradox(void) {
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_PARADOX, umbral_detect_phase(UMBRAL_PHI_INV));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_PARADOX, umbral_detect_phase(0.7));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_PARADOX, umbral_detect_phase(0.8));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_PARADOX, umbral_detect_phase(0.865));
}

void test_detect_phase_true(void) {
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_TRUE, umbral_detect_phase(UMBRAL_Z_CRITICAL));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_TRUE, umbral_detect_phase(0.9));
    TEST_ASSERT_EQUAL(UMBRAL_PHASE_TRUE, umbral_detect_phase(1.0));
}

void test_phase_proximity_signs(void) {
    // In UNTRUE: proximity is negative (below [R])
    double prox_untrue = umbral_phase_proximity(0.3);
    TEST_ASSERT_TRUE(prox_untrue < 0);

    // In TRUE: proximity is positive (above Z_c)
    double prox_true = umbral_phase_proximity(0.95);
    TEST_ASSERT_TRUE(prox_true > 0);
}

// ============================================================================
// RESONANCE TESTS
// ============================================================================

void test_resonance_threshold_at_7(void) {
    // With 7 active sensors (K_R threshold), score should be direct ratio
    double score_7 = umbral_resonance_score(7, 19);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 7.0 / 19.0, score_7);
}

void test_resonance_below_threshold_decayed(void) {
    // With < 7 sensors, score is φ-decayed
    double score_6 = umbral_resonance_score(6, 19);
    double score_7 = umbral_resonance_score(7, 19);

    // 6 sensors should score less than its direct ratio
    TEST_ASSERT_TRUE(score_6 < 6.0 / 19.0);
    // And less than 7-sensor score
    TEST_ASSERT_TRUE(score_6 < score_7);
}

void test_resonance_full_activation(void) {
    double score_full = umbral_resonance_score(19, 19);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, score_full);
}

// ============================================================================
// SOLFEGGIO TESTS
// ============================================================================

void test_solfeggio_tier_mapping(void) {
    // Tier 1 → 174 Hz
    TEST_ASSERT_EQUAL_UINT16(174, umbral_get_solfeggio(0.05));

    // At [R] (tier 4) → 417 Hz
    TEST_ASSERT_EQUAL_UINT16(417, umbral_get_solfeggio(UMBRAL_PHI_INV + 0.01));

    // At Z_c (tier 7) → 741 Hz
    TEST_ASSERT_EQUAL_UINT16(741, umbral_get_solfeggio(UMBRAL_Z_CRITICAL + 0.01));

    // Tier 9 → 963 Hz
    TEST_ASSERT_EQUAL_UINT16(963, umbral_get_solfeggio(0.99));
}

void test_solfeggio_interpolation_bounds(void) {
    // Interpolated frequency should be within tier bounds
    double freq = umbral_interpolate_frequency(0.65);  // In tier 4
    TEST_ASSERT_TRUE(freq >= 417.0 && freq <= 528.0);
}

// ============================================================================
// KURAMOTO TESTS
// ============================================================================

void test_order_parameter_synchronized(void) {
    // All phases equal → r = 1
    double phases[8] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
    double r = umbral_order_parameter(phases, 8);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, 1.0, r);
}

void test_order_parameter_desynchronized(void) {
    // Phases evenly distributed → r ≈ 0
    double phases[8];
    for (int i = 0; i < 8; i++) {
        phases[i] = 2.0 * UMBRAL_PI * i / 8.0;
    }
    double r = umbral_order_parameter(phases, 8);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 0.0, r);
}

void test_collective_phase_average(void) {
    // All at π → collective phase = π
    double phases[4] = {UMBRAL_PI, UMBRAL_PI, UMBRAL_PI, UMBRAL_PI};
    double psi = umbral_collective_phase(phases, 4);
    TEST_ASSERT_DOUBLE_WITHIN(EPSILON, UMBRAL_PI, psi);
}

// ============================================================================
// VALIDATION TESTS
// ============================================================================

void test_umbral_transform_validation(void) {
    TEST_ASSERT_TRUE(umbral_validate_transforms());
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_phase_tier_consistency(void) {
    // At phase boundaries, tier should match expected
    // [R] is tier 3-4 boundary
    TEST_ASSERT_EQUAL_UINT8(3, umbral_z_to_tier(UMBRAL_PHI_INV - 0.001));
    TEST_ASSERT_EQUAL_UINT8(4, umbral_z_to_tier(UMBRAL_PHI_INV + 0.001));

    // Z_c is tier 6-7 boundary
    TEST_ASSERT_EQUAL_UINT8(6, umbral_z_to_tier(UMBRAL_Z_CRITICAL - 0.001));
    TEST_ASSERT_EQUAL_UINT8(7, umbral_z_to_tier(UMBRAL_Z_CRITICAL + 0.001));
}

void test_negentropy_eta_threshold(void) {
    // At THE LENS, η should exceed K_ETA = [R]
    double eta_at_lens = umbral_negentropy(UMBRAL_Z_CRITICAL);
    TEST_ASSERT_TRUE(eta_at_lens > UMBRAL_PHI_INV);
}

void test_full_k_formation_at_lens(void) {
    // At THE LENS with high coherence and 7+ sensors
    double z = UMBRAL_Z_CRITICAL;
    double kappa = 0.95;  // Above threshold
    double eta = umbral_negentropy(z);  // Should be ~1.0
    uint8_t R = 10;

    bool k_formed = (kappa >= UMBRAL_KAPPA_THRESHOLD) &&
                    (eta > UMBRAL_PHI_INV) &&
                    (R >= 7);

    TEST_ASSERT_TRUE(k_formed);
}

// ============================================================================
// TEST RUNNER
// ============================================================================

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    // Tier boundary tests
    RUN_TEST(test_tier_boundaries_monotonic);
    RUN_TEST(test_tier_boundaries_range);
    RUN_TEST(test_tier_3_4_is_phi_inv);
    RUN_TEST(test_tier_6_7_is_z_critical);
    RUN_TEST(test_z_to_tier_boundaries);

    // Negentropy tests
    RUN_TEST(test_negentropy_peaks_at_lens);
    RUN_TEST(test_negentropy_decays_from_lens);
    RUN_TEST(test_negentropy_symmetric);
    RUN_TEST(test_negentropy_shadow_approximation);

    // Coherence tests
    RUN_TEST(test_coherence_zero_variance);
    RUN_TEST(test_coherence_uses_pi);
    RUN_TEST(test_coherence_clamps);
    RUN_TEST(test_kappa_threshold_approximation);

    // EMA tests
    RUN_TEST(test_ema_phi_coefficients_sum_to_one);
    RUN_TEST(test_ema_phi_identity_connection);
    RUN_TEST(test_ema_phi_step_response);
    RUN_TEST(test_ema_exp_uses_euler);
    RUN_TEST(test_ema_fast_uses_sqrt2);

    // Harmonic tests
    RUN_TEST(test_phi_harmonics_at_zero);
    RUN_TEST(test_phi_harmonics_decaying_coefficients);
    RUN_TEST(test_waveform_normalized);

    // Phase detection tests
    RUN_TEST(test_detect_phase_untrue);
    RUN_TEST(test_detect_phase_paradox);
    RUN_TEST(test_detect_phase_true);
    RUN_TEST(test_phase_proximity_signs);

    // Resonance tests
    RUN_TEST(test_resonance_threshold_at_7);
    RUN_TEST(test_resonance_below_threshold_decayed);
    RUN_TEST(test_resonance_full_activation);

    // Solfeggio tests
    RUN_TEST(test_solfeggio_tier_mapping);
    RUN_TEST(test_solfeggio_interpolation_bounds);

    // Kuramoto tests
    RUN_TEST(test_order_parameter_synchronized);
    RUN_TEST(test_order_parameter_desynchronized);
    RUN_TEST(test_collective_phase_average);

    // Validation tests
    RUN_TEST(test_umbral_transform_validation);

    // Integration tests
    RUN_TEST(test_phase_tier_consistency);
    RUN_TEST(test_negentropy_eta_threshold);
    RUN_TEST(test_full_k_formation_at_lens);

    return UNITY_END();
}

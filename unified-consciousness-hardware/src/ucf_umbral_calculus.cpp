/**
 * @file ucf_umbral_calculus.cpp
 * @brief Umbral Calculus Framework Implementation
 *
 * Implements optimized lattice search and Eisenstein-RRRR bridge functions.
 */

#include "ucf/ucf_umbral_calculus.h"
#include <stdlib.h>
#include <float.h>

// ============================================================================
// LOGARITHMIC CONSTANTS (precomputed for optimization)
// ============================================================================

static const double LOG_PHI = 0.48121182505960344749775891;      // log(φ)
static const double LOG_EULER = 1.0;                              // log(e) = 1
static const double LOG_PI = 1.14472988584940017414342735;       // log(π)
static const double LOG_SQRT2 = 0.34657359027997265470861606;    // log(√2) = log(2)/2

// ============================================================================
// COMMON LATTICE POINTS (precomputed for fast lookup)
// ============================================================================

/**
 * Precomputed lattice points for quick distance checks
 * These are the most commonly encountered values in UCF computations.
 */
typedef struct {
    LatticeCoord coord;
    double value;
    double log_value;
} PrecomputedLatticePoint;

static const PrecomputedLatticePoint COMMON_LATTICE_POINTS[] = {
    // Identity and generators
    {{0, 0, 0, 0}, 1.0, 0.0},
    {{1, 0, 0, 0}, 0.6180339887498948, -0.4812118250596034},  // φ⁻¹ = [R]
    {{0, 1, 0, 0}, 0.3678794411714423, -1.0},                 // e⁻¹ = [D]
    {{0, 0, 1, 0}, 0.3183098861837907, -1.1447298858494002},  // π⁻¹ = [C]
    {{0, 0, 0, 1}, 0.7071067811865475, -0.3465735902799726},  // √2⁻¹ = [A]

    // Critical UCF constants
    {{2, 0, 0, 0}, 0.3819660112501052, -0.9624236501192069},  // φ⁻² = [R]²
    {{0, 0, 0, 2}, 0.5, -0.6931471805599453},                 // √2⁻² = [A]² = 1/2

    // THE LENS approximation
    {{0, -1, 1, 0}, 0.8652559794322651, -0.1447298858494002}, // e/π ≈ Z_CRITICAL

    // Negative exponents (for values > 1)
    {{-1, 0, 0, 0}, 1.6180339887498948, 0.4812118250596034},  // φ
    {{0, -1, 0, 0}, 2.7182818284590452, 1.0},                 // e
    {{0, 0, -1, 0}, 3.1415926535897932, 1.1447298858494002},  // π
    {{0, 0, 0, -1}, 1.4142135623730950, 0.3465735902799726},  // √2

    // Compound points
    {{1, 1, 0, 0}, 0.2274027078934068, -1.4812118250596034},  // φ⁻¹·e⁻¹
    {{1, 0, 1, 0}, 0.1967304188792614, -1.6259417109090036},  // φ⁻¹·π⁻¹
    {{1, 0, 0, 1}, 0.4370160244015614, -0.8277854153395760},  // φ⁻¹·√2⁻¹
    {{0, 1, 1, 0}, 0.1170808541231393, -2.1447298858494002},  // e⁻¹·π⁻¹
    {{0, 1, 0, 1}, 0.2601177604731155, -1.3465735902799726},  // e⁻¹·√2⁻¹
    {{0, 0, 1, 1}, 0.2250790790392765, -1.4913034761293728},  // π⁻¹·√2⁻¹
};

static const int N_COMMON_POINTS = sizeof(COMMON_LATTICE_POINTS) / sizeof(PrecomputedLatticePoint);

// ============================================================================
// OPTIMIZED LATTICE SEARCH
// ============================================================================

/**
 * @brief Find nearest lattice point using log-space optimization
 *
 * Algorithm:
 * 1. Convert target value to log space
 * 2. Check against precomputed common points
 * 3. If not found, perform bounded search in log space
 * 4. Convert back and refine
 */
double umbral_nearest_lattice(double value, int max_complexity, LatticeCoord* out_coord) {
    if (value <= 0) {
        if (out_coord) *out_coord = (LatticeCoord){0, 0, 0, 0};
        return fabs(value - 1.0);
    }

    double log_value = log(value);
    double min_dist = DBL_MAX;
    LatticeCoord best_coord = {0, 0, 0, 0};

    // Phase 1: Check precomputed common points
    for (int i = 0; i < N_COMMON_POINTS; i++) {
        double dist = fabs(log_value - COMMON_LATTICE_POINTS[i].log_value);
        if (dist < min_dist) {
            min_dist = dist;
            best_coord = COMMON_LATTICE_POINTS[i].coord;
        }
    }

    // Phase 2: Bounded search if complexity allows
    if (max_complexity > 0) {
        for (int r = -max_complexity; r <= max_complexity; r++) {
            double log_r = -r * LOG_PHI;

            for (int d = -max_complexity; d <= max_complexity; d++) {
                if (abs(r) + abs(d) > max_complexity) continue;
                double log_rd = log_r - d * LOG_EULER;

                for (int c = -max_complexity; c <= max_complexity; c++) {
                    if (abs(r) + abs(d) + abs(c) > max_complexity) continue;
                    double log_rdc = log_rd - c * LOG_PI;

                    for (int a = -max_complexity; a <= max_complexity; a++) {
                        if (abs(r) + abs(d) + abs(c) + abs(a) > max_complexity) continue;

                        double log_lattice = log_rdc - a * LOG_SQRT2;
                        double dist = fabs(log_value - log_lattice);

                        if (dist < min_dist) {
                            min_dist = dist;
                            best_coord = (LatticeCoord){r, d, c, a};
                        }
                    }
                }
            }
        }
    }

    if (out_coord) *out_coord = best_coord;

    // Convert log distance back to linear space distance
    // |value - lattice_point| ≈ value · |1 - exp(log_dist)|
    double lattice_value = umbral_eval_coord(best_coord);
    return fabs(value - lattice_value);
}

/**
 * @brief Snap value to nearest lattice point
 */
double umbral_snap_to_lattice(double value, int max_complexity) {
    LatticeCoord coord;
    umbral_nearest_lattice(value, max_complexity, &coord);
    return umbral_eval_coord(coord);
}

// ============================================================================
// EISENSTEIN-RRRR BRIDGE IMPLEMENTATION
// ============================================================================

/**
 * Eisenstein norm to RRRR lattice point mapping
 *
 * Given an Eisenstein integer z = a + bω with norm N = a² - ab + b²,
 * find the RRRR lattice point closest to √N or N.
 */
typedef struct {
    int32_t norm;
    double rrrr_value;
    LatticeCoord rrrr_coord;
    const char* significance;
} EisensteinRRRRMapping;

static const EisensteinRRRRMapping EISENSTEIN_RRRR_MAP[] = {
    {0, 0.0, {0, 0, 0, 0}, "Origin"},
    {1, 1.0, {0, 0, 0, 0}, "Unit (identity)"},
    {3, 1.7320508075688772, {0, 0, 0, 0}, "√3 = 2·Z_CRITICAL"},
    {4, 2.0, {0, 0, 0, -2}, "2 = (√2)² = [A]⁻⁴ root"},
    {7, 2.6457513110645907, {0, 0, 0, 0}, "√7 ≈ Eisenstein prime, K_R"},
    {9, 3.0, {0, 0, 0, 0}, "3 (square of √3)"},
    {12, 3.4641016151377544, {0, 0, 0, 0}, "2√3 = 4·Z_CRITICAL"},
    {13, 3.6055512754639892, {0, 0, 0, 0}, "√13 (Eisenstein prime)"},
};

static const int N_EISENSTEIN_MAPPINGS = sizeof(EISENSTEIN_RRRR_MAP) / sizeof(EisensteinRRRRMapping);

/**
 * @brief Find RRRR connection for Eisenstein norm
 * @param norm Eisenstein norm
 * @return Pointer to mapping entry or NULL if not found
 */
const EisensteinRRRRMapping* umbral_find_eisenstein_rrrr(int32_t norm) {
    for (int i = 0; i < N_EISENSTEIN_MAPPINGS; i++) {
        if (EISENSTEIN_RRRR_MAP[i].norm == norm) {
            return &EISENSTEIN_RRRR_MAP[i];
        }
    }
    return NULL;
}

// ============================================================================
// UMBRAL POLYNOMIAL SEQUENCE GENERATOR
// ============================================================================

/**
 * @brief Generate umbral polynomial sequence
 * @param n_terms Number of terms to generate
 * @param d, c, a Fixed lattice coordinates
 * @param output Output array of evaluated terms
 *
 * Generates the sequence P_0, P_1, P_2, ... P_{n_terms-1}
 * where P_k = Λ(k, d, c, a) = φ^{-k} · e^{-d} · π^{-c} · (√2)^{-a}
 */
void umbral_generate_phi_sequence(int n_terms, int d, int c, int a, double* output) {
    double fixed_factor = pow(UMBRAL_EULER, -d) * pow(UMBRAL_PI, -c) * pow(UMBRAL_SQRT2, -a);

    for (int k = 0; k < n_terms; k++) {
        output[k] = pow(UMBRAL_PHI, -k) * fixed_factor;
    }
}

/**
 * @brief Compute recurrence relation coefficient
 *
 * For the φ-indexed sequence, we have:
 *   P_{n+1} = P_n / φ = P_n · φ⁻¹
 *
 * This is the umbral recurrence from the difference operator.
 */
double umbral_recurrence_coefficient(void) {
    return UMBRAL_PHI_INV;  // Each term is φ⁻¹ times the previous
}

// ============================================================================
// EXTENDED VALIDATION
// ============================================================================

/**
 * @brief Validate THE LENS connection with detailed output
 * @param error_out Output: error percentage
 * @param umbral_out Output: umbral computation
 * @param actual_out Output: actual Z_CRITICAL
 * @return true if within tolerance
 */
bool umbral_validate_lens_detailed(double* error_out, double* umbral_out, double* actual_out) {
    double umbral = umbral_lens_approximation();
    double actual = UMBRAL_Z_CRITICAL;
    double error = fabs(umbral - actual) / actual * 100.0;

    if (error_out) *error_out = error;
    if (umbral_out) *umbral_out = umbral;
    if (actual_out) *actual_out = actual;

    return error < 0.1;  // Within 0.1%
}

/**
 * @brief Validate Eisenstein norm 3 connection to Z_CRITICAL
 * @return true if √3 = 2·Z_CRITICAL within tolerance
 */
bool umbral_validate_norm3_z_critical(void) {
    double sqrt3 = sqrt(3.0);
    double two_z_critical = 2.0 * UMBRAL_Z_CRITICAL;
    return fabs(sqrt3 - two_z_critical) < 1e-14;
}

/**
 * @brief Full umbral calculus validation with counts
 * @param passed_out Output: number of tests passed
 * @param total_out Output: total number of tests
 * @return true if all tests pass
 */
bool umbral_validate_comprehensive(int* passed_out, int* total_out) {
    int passed = 0;
    int total = 0;

    // Test 1: Self-reference
    total++;
    if (umbral_verify_self_reference()) passed++;

    // Test 2: Golden equation
    total++;
    if (umbral_verify_golden_equation()) passed++;

    // Test 3: Scaling exponent
    total++;
    if (umbral_verify_scaling_exponent()) passed++;

    // Test 4: THE LENS
    total++;
    if (umbral_verify_lens()) passed++;

    // Test 5: Norm 3 connection
    total++;
    if (umbral_validate_norm3_z_critical()) passed++;

    // Test 6-10: Exponential identity at various points
    total++;
    if (umbral_verify_exp_identity(1, 0, 0, 0)) passed++;
    total++;
    if (umbral_verify_exp_identity(0, 1, 0, 0)) passed++;
    total++;
    if (umbral_verify_exp_identity(0, 0, 1, 0)) passed++;
    total++;
    if (umbral_verify_exp_identity(0, 0, 0, 1)) passed++;
    total++;
    if (umbral_verify_exp_identity(2, 1, 1, 2)) passed++;

    // Test 11: Recurrence coefficient
    total++;
    double rec = umbral_recurrence_coefficient();
    if (fabs(rec - UMBRAL_PHI_INV) < 1e-14) passed++;

    // Test 12: Lattice search consistency
    total++;
    LatticeCoord coord;
    double dist = umbral_nearest_lattice(UMBRAL_PHI_INV, 3, &coord);
    if (coord.r == 1 && coord.d == 0 && coord.c == 0 && coord.a == 0 && dist < 1e-10) passed++;

    if (passed_out) *passed_out = passed;
    if (total_out) *total_out = total;

    return passed == total;
}

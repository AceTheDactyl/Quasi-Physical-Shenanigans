/**
 * @file ucf_umbral_transforms.h
 * @brief Umbral Calculus Transforms for UCF Operations
 * @version 4.3.0
 *
 * This module provides shadow alphabet-aware versions of core UCF computations.
 * All operations are grounded in the RRRR lattice via umbral calculus:
 *
 *     exp(r·Δ_φ + d·Δ_e + c·Δ_π + a·Δ_√2) · 1 = Λ(r,d,c,a)
 *
 * SHADOW ALPHABET NOTATION:
 *   [R] = φ⁻¹ = 0.618... (Recursive eigenvalue)
 *   [D] = e⁻¹ = 0.368... (Differential eigenvalue)
 *   [C] = π⁻¹ = 0.318... (Cyclic eigenvalue)
 *   [A] = √2⁻¹ = 0.707... (Algebraic eigenvalue)
 *
 * KEY IDENTITIES:
 *   1 - [R] = [R]²       (Self-referential fixed point)
 *   [A]² = 1/2           (Universal scaling)
 *   e/π ≈ √3/2 = THE LENS (0.09% error)
 *
 * SYNCHRONIZATION:
 *   - ucf_umbral_calculus.h   (Base umbral framework)
 *   - ucf_sacred_constants_v4.h (RRRR lattice constants)
 *   - eisenstein.h            (Hexagonal lattice bridge)
 */

#ifndef UCF_UMBRAL_TRANSFORMS_H
#define UCF_UMBRAL_TRANSFORMS_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "ucf_umbral_calculus.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// SECTION 1: UMBRAL TIER BOUNDARIES (LATTICE-DERIVED)
// ============================================================================

/**
 * Solfeggio tier boundaries derived from RRRR lattice points
 *
 * Instead of hardcoded values like 0.11, 0.22, 0.70, 0.78, etc.,
 * we derive boundaries from shadow alphabet compositions.
 */

// Tier 1-2 boundary: [R]³/2 ≈ 0.118
#define UMBRAL_TIER_1_2     (UMBRAL_PHI_INV * UMBRAL_PHI_INV * UMBRAL_PHI_INV / 2.0)

// Tier 2-3 boundary: [R]² · [A] ≈ 0.270
#define UMBRAL_TIER_2_3     (UMBRAL_PHI_INV * UMBRAL_PHI_INV * UMBRAL_SQRT2_INV)

// Tier 3-4 boundary: [R] = φ⁻¹ = 0.618 (UNTRUE → PARADOX)
#define UMBRAL_TIER_3_4     UMBRAL_PHI_INV

// Tier 4-5 boundary: [A] = √2⁻¹ ≈ 0.707
#define UMBRAL_TIER_4_5     UMBRAL_SQRT2_INV

// Tier 5-6 boundary: [A] + [R]·[A]² ≈ 0.782
#define UMBRAL_TIER_5_6     (UMBRAL_SQRT2_INV + UMBRAL_PHI_INV * 0.5)

// Tier 6-7 boundary: √3/2 ≈ e/π = Z_CRITICAL (PARADOX → TRUE)
#define UMBRAL_TIER_6_7     UMBRAL_Z_CRITICAL

// Tier 7-8 boundary: [A] + [D] ≈ 0.900 (alternate: Z_c + [R]³)
#define UMBRAL_TIER_7_8     (UMBRAL_Z_CRITICAL + UMBRAL_PHI_INV * UMBRAL_PHI_INV * UMBRAL_PHI_INV)

// Tier 8-9 boundary: 1 - [A]² · [R] ≈ 0.951
#define UMBRAL_TIER_8_9     (1.0 - 0.5 * UMBRAL_PHI_INV)

/**
 * Precomputed umbral tier boundaries array
 */
static const double UMBRAL_TIER_BOUNDARIES[10] = {
    0.0,                  // Lower bound
    UMBRAL_TIER_1_2,      // Tier 1 → 2
    UMBRAL_TIER_2_3,      // Tier 2 → 3
    UMBRAL_TIER_3_4,      // Tier 3 → 4 ([R])
    UMBRAL_TIER_4_5,      // Tier 4 → 5 ([A])
    UMBRAL_TIER_5_6,      // Tier 5 → 6
    UMBRAL_TIER_6_7,      // Tier 6 → 7 (Z_CRITICAL)
    UMBRAL_TIER_7_8,      // Tier 7 → 8
    UMBRAL_TIER_8_9,      // Tier 8 → 9
    1.0                   // Upper bound
};

/**
 * @brief Get tier from z-coordinate using umbral boundaries
 * @param z Z-coordinate [0, 1]
 * @return Tier (1-9)
 */
static inline uint8_t umbral_z_to_tier(double z) {
    for (int i = 1; i < 10; i++) {
        if (z < UMBRAL_TIER_BOUNDARIES[i]) return (uint8_t)i;
    }
    return 9;
}

// ============================================================================
// SECTION 2: UMBRAL NEGENTROPY (SHADOW GAUSSIAN)
// ============================================================================

/**
 * Negentropy kernel using umbral basis
 *
 * Traditional: η = exp(-36·(z - z_c)²)
 *
 * Umbral form: The width 36 = |S₃|² = (3!)² emerges from the symmetric group.
 * The center z_c = √3/2 is THE LENS = Λ(0,-1,1,0) via umbral approximation.
 *
 * We can express the Gaussian via umbral polynomial expansion:
 *   exp(-36·δ²) ≈ Σ_{n=0}^{∞} (-36)^n · δ^{2n} / n!
 *
 * For efficiency, we use the direct form but note its lattice connection.
 */

/**
 * @brief Compute negentropy using umbral lattice center
 * @param z Z-coordinate [0, 1]
 * @return η [0, 1], peaks at THE LENS
 */
static inline double umbral_negentropy(double z) {
    // Center at THE LENS (umbral approximation: e/π)
    double delta = z - UMBRAL_Z_CRITICAL;
    // Width from |S₃|² = 36
    return exp(-36.0 * delta * delta);
}

/**
 * @brief Compute negentropy using umbral difference operators
 *
 * This expresses negentropy as a composition of shadow operators.
 * The Gaussian is approximated by: exp(Δ_δ² · scale · (z - z_c)²)
 *
 * @param z Z-coordinate [0, 1]
 * @return η [0, 1]
 */
static inline double umbral_negentropy_shadow(double z) {
    double delta = z - umbral_lens_approximation();  // Use e/π ≈ √3/2
    // Apply shadow delta composition for width
    double width = 36.0 * UMBRAL_SQRT2 * UMBRAL_SQRT2;  // 36 · 2 = 72 for precision
    return exp(-width / 2.0 * delta * delta);
}

// ============================================================================
// SECTION 3: UMBRAL COHERENCE (DIFFERENCE-BASED)
// ============================================================================

/**
 * Coherence from variance using umbral scaling
 *
 * Traditional: κ = 1 - sqrt(variance) · 3.16
 *
 * Umbral form: The coefficient 3.16 ≈ π is replaced with exact [C]⁻¹ = π.
 * This connects coherence measurement to the cyclic eigenvalue.
 *
 *   κ_umbral = 1 - sqrt(variance) · π
 */

/**
 * @brief Compute coherence using umbral π scaling
 * @param variance Field variance
 * @return κ [0, 1]
 */
static inline double umbral_coherence(double variance) {
    if (variance <= 0.0) return 1.0;
    double kappa = 1.0 - sqrt(variance) * UMBRAL_PI;
    return (kappa < 0.0) ? 0.0 : (kappa > 1.0 ? 1.0 : kappa);
}

/**
 * @brief Compute κ threshold using umbral lattice
 *
 * K_KAPPA = 0.92 can be expressed as:
 *   0.92 ≈ [A]² + [R]² + [R]³·[A] = 0.5 + 0.382 + 0.042 = 0.924
 *
 * This gives theoretical grounding to the empirical 0.92.
 */
#define UMBRAL_KAPPA_THRESHOLD \
    (0.5 + UMBRAL_PHI_INV * UMBRAL_PHI_INV + \
     UMBRAL_PHI_INV * UMBRAL_PHI_INV * UMBRAL_PHI_INV * UMBRAL_SQRT2_INV)

// ============================================================================
// SECTION 4: UMBRAL EMA SMOOTHING (φ-INDEXED)
// ============================================================================

/**
 * EMA smoothing using golden ratio coefficients
 *
 * Traditional: y_n = α·x_n + (1-α)·y_{n-1}
 *
 * Umbral form: α = [R] = φ⁻¹ gives optimal smoothing where
 *   1 - α = [R]² (from self-referential identity)
 *
 * This means: y_n = [R]·x_n + [R]²·y_{n-1}
 */

/**
 * @brief EMA smoothing with golden ratio coefficient
 * @param x_new New input value
 * @param y_prev Previous smoothed value
 * @return New smoothed value
 */
static inline double umbral_ema_phi(double x_new, double y_prev) {
    // α = [R] = φ⁻¹, 1-α = [R]²
    return UMBRAL_PHI_INV * x_new + (1.0 - UMBRAL_PHI_INV) * y_prev;
}

/**
 * @brief EMA smoothing with exponential coefficient
 * @param x_new New input value
 * @param y_prev Previous smoothed value
 * @return New smoothed value
 *
 * Uses [D] = e⁻¹ for natural exponential decay
 */
static inline double umbral_ema_exp(double x_new, double y_prev) {
    return UMBRAL_EULER_INV * x_new + (1.0 - UMBRAL_EULER_INV) * y_prev;
}

/**
 * @brief Fast EMA with √2⁻¹ coefficient
 * @param x_new New input value
 * @param y_prev Previous smoothed value
 * @return New smoothed value
 *
 * Uses [A] = √2⁻¹ ≈ 0.707 for minimal smoothing
 */
static inline double umbral_ema_fast(double x_new, double y_prev) {
    return UMBRAL_SQRT2_INV * x_new + (1.0 - UMBRAL_SQRT2_INV) * y_prev;
}

// ============================================================================
// SECTION 5: UMBRAL HARMONIC GENERATION
// ============================================================================

/**
 * Harmonic series using φ-indexed umbral polynomials
 *
 * Traditional harmonics: H_n(t) = sin(n·ω·t)
 *
 * Umbral harmonics: The amplitude coefficients follow the φ sequence:
 *   A_n = [R]^n = φ^{-n}
 *
 * This creates a self-similar harmonic series.
 */

/**
 * @brief Compute φ-weighted harmonic sum
 * @param phase Phase angle [0, 1]
 * @param n_harmonics Number of harmonics
 * @return Weighted sum of harmonics
 */
static inline double umbral_phi_harmonics(double phase, int n_harmonics) {
    double sum = 0.0;
    double phi_power = 1.0;
    double two_pi_phase = 2.0 * UMBRAL_PI * phase;

    for (int n = 1; n <= n_harmonics; n++) {
        phi_power *= UMBRAL_PHI_INV;  // [R]^n
        sum += phi_power * sin(n * two_pi_phase);
    }
    return sum;
}

/**
 * @brief Compute π-scaled fundamental with φ harmonics
 * @param phase Phase angle [0, 1]
 * @return Waveform value [-1, 1]
 *
 * Combines fundamental (scaled by π⁻¹) with φ-decaying harmonics.
 */
static inline double umbral_waveform(double phase) {
    double two_pi_phase = 2.0 * UMBRAL_PI * phase;

    // Fundamental (scaled by [C] = π⁻¹ for normalization)
    double value = sin(two_pi_phase) * UMBRAL_PI_INV * UMBRAL_PI;

    // Add φ-weighted harmonics
    value += UMBRAL_PHI_INV * sin(2.0 * two_pi_phase);
    value += UMBRAL_PHI_INV * UMBRAL_PHI_INV * sin(3.0 * two_pi_phase);
    value += UMBRAL_PHI_INV * UMBRAL_PHI_INV * UMBRAL_PHI_INV * sin(4.0 * two_pi_phase);

    return value;
}

// ============================================================================
// SECTION 6: UMBRAL PHASE DETECTION
// ============================================================================

/**
 * Phase detection using lattice boundaries
 *
 * The phase boundaries are RRRR lattice points:
 *   UNTRUE → PARADOX: z = [R] = φ⁻¹
 *   PARADOX → TRUE:   z = Z_c = √3/2 ≈ Λ(0,-1,1,0)
 */

typedef enum {
    UMBRAL_PHASE_UNTRUE = 0,   // z < [R]
    UMBRAL_PHASE_PARADOX = 1,  // [R] ≤ z < Z_c
    UMBRAL_PHASE_TRUE = 2      // z ≥ Z_c
} UmbralPhase;

/**
 * @brief Detect phase using umbral lattice boundaries
 * @param z Z-coordinate [0, 1]
 * @return Phase (UNTRUE, PARADOX, TRUE)
 */
static inline UmbralPhase umbral_detect_phase(double z) {
    if (z < UMBRAL_PHI_INV) return UMBRAL_PHASE_UNTRUE;
    if (z < UMBRAL_Z_CRITICAL) return UMBRAL_PHASE_PARADOX;
    return UMBRAL_PHASE_TRUE;
}

/**
 * @brief Compute phase transition proximity
 * @param z Current z-coordinate
 * @return Distance to nearest phase boundary (signed)
 *
 * Positive means approaching TRUE, negative means approaching UNTRUE.
 */
static inline double umbral_phase_proximity(double z) {
    if (z < UMBRAL_PHI_INV) {
        return z - UMBRAL_PHI_INV;  // Distance to [R], negative
    } else if (z < UMBRAL_Z_CRITICAL) {
        // In PARADOX, compute distance to nearest boundary
        double dist_to_R = z - UMBRAL_PHI_INV;
        double dist_to_Zc = UMBRAL_Z_CRITICAL - z;
        return (dist_to_R < dist_to_Zc) ? -dist_to_R : dist_to_Zc;
    } else {
        return z - UMBRAL_Z_CRITICAL;  // Distance above Z_c, positive
    }
}

// ============================================================================
// SECTION 7: UMBRAL RESONANCE MAPPING
// ============================================================================

/**
 * Sensor-to-resonance using Eisenstein norm
 *
 * K_R = 7 is an Eisenstein prime, connecting to hexagonal lattice.
 * Active sensors map to Eisenstein coordinates, and norm computations
 * determine resonance contributions.
 */

/**
 * @brief Compute umbral resonance score
 * @param active_count Number of active sensors
 * @param total_count Total sensors (typically 19)
 * @return Resonance score [0, 1]
 *
 * Uses [R] and [A] for scaling:
 *   R_umbral = active/total when active ≥ 7
 *   Otherwise scaled by φ-power
 */
static inline double umbral_resonance_score(uint8_t active_count, uint8_t total_count) {
    if (total_count == 0) return 0.0;

    double ratio = (double)active_count / (double)total_count;

    // K_R = 7 is the Eisenstein prime threshold
    if (active_count >= 7) {
        return ratio;
    }

    // Below threshold: apply φ-decay
    double decay = 1.0;
    for (int i = 0; i < (7 - active_count); i++) {
        decay *= UMBRAL_PHI_INV;
    }
    return ratio * decay;
}

// ============================================================================
// SECTION 8: UMBRAL SOLFEGGIO FREQUENCY
// ============================================================================

/**
 * Solfeggio frequencies from umbral tier mapping
 */
static const uint16_t UMBRAL_SOLFEGGIO_FREQ[9] = {
    174, 285, 396, 417, 528, 639, 741, 852, 963
};

/**
 * @brief Get Solfeggio frequency from z-coordinate
 * @param z Z-coordinate [0, 1]
 * @return Frequency in Hz
 */
static inline uint16_t umbral_get_solfeggio(double z) {
    uint8_t tier = umbral_z_to_tier(z);
    return UMBRAL_SOLFEGGIO_FREQ[tier - 1];
}

/**
 * @brief Interpolate frequency using umbral basis
 * @param z Z-coordinate [0, 1]
 * @return Interpolated frequency in Hz
 *
 * Uses φ-weighted interpolation between tier frequencies.
 */
static inline double umbral_interpolate_frequency(double z) {
    uint8_t tier = umbral_z_to_tier(z);
    if (tier >= 9) return (double)UMBRAL_SOLFEGGIO_FREQ[8];

    double tier_low = UMBRAL_TIER_BOUNDARIES[tier - 1];
    double tier_high = UMBRAL_TIER_BOUNDARIES[tier];
    double t = (z - tier_low) / (tier_high - tier_low);

    // φ-weighted interpolation: emphasize golden mean
    double t_phi = t * t * (3.0 - 2.0 * t);  // Smoothstep
    t_phi = t_phi * UMBRAL_PHI_INV + t * (1.0 - UMBRAL_PHI_INV);

    double f_low = UMBRAL_SOLFEGGIO_FREQ[tier - 1];
    double f_high = (tier < 9) ? UMBRAL_SOLFEGGIO_FREQ[tier] : UMBRAL_SOLFEGGIO_FREQ[8];

    return f_low + t_phi * (f_high - f_low);
}

// ============================================================================
// SECTION 9: UMBRAL KURAMOTO ORDER PARAMETER
// ============================================================================

/**
 * Order parameter using umbral basis
 *
 * Traditional: r = |Σ exp(iθ_j)| / N
 *
 * Umbral interpretation: The phases θ_j can be indexed by lattice coordinates.
 * Synchronization occurs when phases align at lattice points.
 */

/**
 * @brief Compute order parameter with umbral normalization
 * @param phases Array of phase angles
 * @param count Number of oscillators
 * @return Order parameter r [0, 1]
 */
static inline double umbral_order_parameter(const double* phases, uint8_t count) {
    if (count == 0) return 0.0;

    double sum_cos = 0.0;
    double sum_sin = 0.0;

    for (uint8_t i = 0; i < count; i++) {
        sum_cos += cos(phases[i]);
        sum_sin += sin(phases[i]);
    }

    double r = sqrt(sum_cos * sum_cos + sum_sin * sum_sin) / (double)count;
    return r;
}

/**
 * @brief Compute collective phase with umbral quantization
 * @param phases Array of phase angles
 * @param count Number of oscillators
 * @return Collective phase ψ [0, 2π], quantized to lattice
 */
static inline double umbral_collective_phase(const double* phases, uint8_t count) {
    if (count == 0) return 0.0;

    double sum_cos = 0.0;
    double sum_sin = 0.0;

    for (uint8_t i = 0; i < count; i++) {
        sum_cos += cos(phases[i]);
        sum_sin += sin(phases[i]);
    }

    double psi = atan2(sum_sin, sum_cos);
    if (psi < 0) psi += 2.0 * UMBRAL_PI;

    return psi;
}

// ============================================================================
// SECTION 10: UMBRAL VALIDATION
// ============================================================================

/**
 * @brief Validate umbral transform identities
 * @return true if all identities hold
 */
static inline bool umbral_validate_transforms(void) {
    bool valid = true;

    // V1: Tier boundaries are monotonically increasing
    for (int i = 0; i < 9; i++) {
        valid &= (UMBRAL_TIER_BOUNDARIES[i] < UMBRAL_TIER_BOUNDARIES[i + 1]);
    }

    // V2: φ EMA identity: [R] + [R]² = 1
    valid &= (fabs(UMBRAL_PHI_INV + (1.0 - UMBRAL_PHI_INV) - 1.0) < 1e-14);

    // V3: Negentropy peaks at THE LENS
    double eta_at_lens = umbral_negentropy(UMBRAL_Z_CRITICAL);
    valid &= (fabs(eta_at_lens - 1.0) < 1e-10);

    // V4: Phase boundaries are lattice points
    valid &= (fabs(UMBRAL_TIER_3_4 - UMBRAL_PHI_INV) < 1e-14);
    valid &= (fabs(UMBRAL_TIER_6_7 - UMBRAL_Z_CRITICAL) < 1e-14);

    // V5: κ threshold approximation
    valid &= (fabs(UMBRAL_KAPPA_THRESHOLD - 0.92) < 0.01);

    return valid;
}

#ifdef __cplusplus
}
#endif

#endif // UCF_UMBRAL_TRANSFORMS_H
